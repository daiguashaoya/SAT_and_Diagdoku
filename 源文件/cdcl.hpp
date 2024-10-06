#ifndef cdcl_HPP
#define cdcl_HPP
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace cdcl {

// 一些类型定义，方便后续代码使用
using Var = int;       // 变量类型
using CRef = int;      // 子句引用类型
using lbool = int;     // 布尔值类型

// 常量定义，CRef_Undef表示未定义的子句引用
const CRef CRef_Undef = -1;

class Solver2 {
 private:
  // 定义布尔值状态 l_True, l_False, l_Undef
  const lbool l_True = 0;    // 表示真
  const lbool l_False = 1;   // 表示假
  const lbool l_Undef = 2;   // 表示未定义

  const int var_Undef = -1;  // 表示未定义的变量

  // Literal（文字）的结构体定义
  struct Lit {
    int x; // 文字存储的值

    // 重载运算符，方便比较和操作文字
    inline bool operator==(Lit p) const { return x == p.x; }
    inline bool operator!=(Lit p) const { return x != p.x; }
    inline bool operator<(Lit p) const { return x < p.x; }

    // 取反操作符，翻转文字的符号
    inline Lit operator~() {
      Lit q;
      q.x = x ^ 1; // 通过位运算翻转符号
      return q;
    }
  };

  // 创建一个文字，传入变量和符号
  inline Lit mkLit(Var var, bool sign) {
    Lit p;
    p.x = var + var + sign;  // 根据变量和符号创建文字
    return p;
  }

  // 获取文字的符号
  inline bool sign(Lit p) const { return p.x & 1; }

  // 获取文字的变量部分
  inline int var(Lit p) const { return p.x >> 1; }

  // 将变量转换为整型
  inline int toInt(Var v) { return v; }

  // 将文字转换为整型
  inline int toInt(Lit p) { return p.x; }

  // 将整型转换为文字
  inline Lit toLit(int x) {
    Lit p;
    p.x = x;
    return p;
  }

  // 定义两个特殊的文字
  const Lit lit_Undef = {-2}; // 未定义的文字
  const Lit lit_Error = {-1}; // 错误的文字

  // VarData（变量数据）的结构体定义
  struct VarData {
    CRef reason;  // 子句引用，表示导致该变量的推理来源
    int level;    // 变量所属的决策层次
  };

  // 创建变量数据的函数
  inline VarData mkVarData(CRef cr, int l) {
    VarData d = {cr, l};
    return d;
  }

  // Watcher（监视器）的结构体定义，用于子句的两文字监视机制
  struct Watcher {
    CRef cref;    // 子句引用
    Lit blocker;  // 被阻挡的文字
    Watcher() {}
    Watcher(CRef cr, Lit p) : cref(cr), blocker(p) {}

    // 重载比较运算符
    bool operator==(const Watcher &w) const { return cref == w.cref; }
    bool operator!=(const Watcher &w) const { return cref != w.cref; }
  };

  // Clause（子句）的类定义，表示一个子句
  class Clause {
   public:
    // 子句头部，包含学习状态和子句大小
    struct {
      bool learnt;  // 是否为学习子句
      int size;     // 子句中的文字数量
    } header;

    // 存储子句中的文字
    std::vector<Lit> data;

    Clause() {}

    // 构造函数，根据文字列表和学习状态创建子句
    Clause(const std::vector<Lit> &ps, bool learnt) {
      header.learnt = learnt;    // 设置是否为学习子句
      header.size = ps.size();   // 设置子句大小

      data.resize(header.size);  // 调整存储空间大小
      for (int i = 0; i < ps.size(); i++) {
        data[i] = ps[i];         // 拷贝文字到数据中
      }
    }

    // 获取子句大小
    int size() const { return header.size; }

    // 检查是否为学习子句
    bool learnt() const { return header.learnt; }

    // 子句的下标运算符，获取或设置文字
    Lit &operator[](int i) { return data[i]; }
    Lit operator[](int i) const { return data[i]; }
  };

  // 分配子句的函数，将文字列表转化为子句并分配其内存
  CRef allocClause(std::vector<Lit> &ps, bool learnt = false) {
    static CRef res = 0;         // 静态变量保存子句引用
    ca[res] = std::move(Clause(ps, learnt));  // 创建子句
    return res++;                // 返回子句引用并递增
  }

  // 创建新变量，添加到各个数据结构中
  Var newVar(bool sign = true, bool dvar = true) {
    int v = nVars(); // 获取当前变量的数量

    // 添加未定义布尔值到变量分配列表
    assigns.emplace_back(l_Undef);

    // 创建变量数据，推理来源未定义，层次为0
    vardata.emplace_back(mkVarData(CRef_Undef, 0));

    // 将活动性、极性等信息初始化
    activity.emplace_back(0.0);
    seen.push_back(false);
    polarity.push_back(sign);
    decision.push_back(0);

    // 设置变量为决策变量
    setDecisionVar(v, dvar);
    return v;
  }

  // 添加一个子句
  bool addClause_(std::vector<Lit> &ps) {
    // 如果子句为空，返回false
    if (ps.size() == 0) {
      return false;
    } else if (ps.size() == 1) {  // 如果子句只有一个文字，直接入队
      uncheckedEnqueue(ps[0]);
    } else {  // 否则将子句分配并附加到监视列表
      CRef cr = allocClause(ps, false);
      attachClause(cr);
    }
    return true;
  }

  // 将子句附加到两文字监视列表
  void attachClause(CRef cr) {
    const Clause &c = ca[cr]; // 获取子句

    assert(c.size() > 1);     // 确保子句长度大于1

    // 将子句的前两个文字添加到监视列表
    watches[(~c[0]).x].emplace_back(Watcher(cr, c[1]));
    watches[(~c[1]).x].emplace_back(Watcher(cr, c[0]));
  }

  // 解析子句的函数，从字符串中读取子句并转化为文字列表
  void readClause(const std::string &line, std::vector<Lit> &lits) {
    lits.clear();  // 清空文字列表
    int parsed_lit, var;
    parsed_lit = var = 0;  // 初始化

    bool neg = false;      // 记录是否为负文字
    std::stringstream ss(line);  // 创建字符串流读取每行

    while (ss) {  // 逐个读取文字
      int val;
      ss >> val;  // 从字符串中读取一个值
      if (val == 0) break;  // 如果读取到0，则结束
      var = abs(val) - 1;   // 获取变量编号

      while (var >= nVars()) {  // 如果变量编号超出范围，创建新变量
        newVar();
      }
      // 根据文字正负号创建文字，并添加到列表
      lits.emplace_back(val > 0 ? mkLit(var, false) : mkLit(var, true));
    }
  }


  // 维护子句集合
std::unordered_map<CRef, Clause> ca;  // 存储所有子句，CRef为子句引用，Clause为子句对象
std::unordered_set<CRef> clauses;     // 原始问题中的子句集合
std::unordered_set<CRef> learnts;     // 学习得到的子句集合

// 监视器列表，每个文字都监视它的两个文字
std::unordered_map<int, std::vector<Watcher>> watches; 

// 变量相关数据
std::vector<VarData> vardata;  // 存储每个变量的推理来源和决策层次
std::vector<bool> polarity;    // 每个变量的首选极性（布尔值）
std::vector<bool> decision;    // 标记是否为决策变量
std::vector<bool> seen;        // 用于标记是否访问过某个变量

// 追踪已分配的文字（赋值）以及它们的决策层次
int qhead;                     // trail队列的当前头部指针
std::vector<Lit> trail;        // 存储已分配的文字
std::vector<int> trail_lim;    // 存储每个决策层次的trail边界

// 决策变量的优先级队列（使用活动性排序）
std::set<std::pair<double, Var>> order_heap;  // 使用变量活动性和变量编号进行排序
std::vector<double> activity;  // 每个变量的活动性
double var_inc;                // 活动性的增加步长

// 模型和冲突相关
std::vector<Lit> model;        // 存储最终的解（模型）
std::vector<Lit> conflict;     // 存储当前冲突的子句

// 获取当前的变量数量（nVars）
int nVars() const { return vardata.size(); }  // 返回变量数量

// 获取当前决策层次
int decisionLevel() const { return trail_lim.size(); }  // 决策层次与trail_lim大小相同

// 新增一个决策层次
void newDecisionLevel() { trail_lim.emplace_back(trail.size()); }  // 在当前trail边界记录新的决策层次

// 获取某个变量的推理来源子句
inline CRef reason(Var x) const { return vardata[x].reason; }

// 获取某个变量的决策层次
inline int level(Var x) const { return vardata[x].level; }

// 变量活动性增加函数，增加某个变量的活动性
inline void varBumpActivity(Var v) {
    std::pair<double, Var> p = std::make_pair(activity[v], v);
    activity[v] += var_inc;  // 增加变量的活动性
    // 如果该变量在order_heap中，移除旧的值并插入新的活动性
    if (order_heap.erase(p) == 1) {
      order_heap.emplace(std::make_pair(activity[v], v));
    }

    // 如果活动性过大，则对所有活动性进行缩放（防止溢出）
    if (activity[v] > 1e100) {
      std::set<std::pair<double, Var>> tmp_order;
      tmp_order = std::move(order_heap);
      order_heap.clear();
      for (int i = 0; i < nVars(); i++) {
        activity[i] *= 1e-100;  // 缩小所有变量的活动性
      }
      for (auto &val : tmp_order) {
        order_heap.emplace(std::make_pair(activity[val.second], val.second));  // 重新插入活动性缩放后的变量
      }
      var_inc *= 1e-100;  // 缩小活动性增加步长
    }
}

// 判断子句是否已经满足
bool satisfied(const Clause &c) const {
    for (int i = 0; i < c.size(); i++) {
        if (value(c[i]) == l_True) {  // 如果某个文字为真，则子句已经满足
            return true;
        }
    }
    return false;  // 否则，子句未满足
}

// 获取某个变量的当前赋值（布尔值）
lbool value(Var p) const { return assigns[p]; }

// 获取某个文字的当前赋值
lbool value(Lit p) const {
    if (assigns[var(p)] == l_Undef) {
        return l_Undef;  // 如果文字未赋值，返回未定义
    }
    return assigns[var(p)] ^ sign(p);  // 返回文字的值，考虑符号
}

// 设置某个变量为决策变量
void setDecisionVar(Var v, bool b) {
    decision[v] = b;  // 设置该变量为决策变量
    order_heap.emplace(std::make_pair(0.0, v));  // 将变量添加到决策队列中
}

// 将某个文字未检查入队（赋值）
void uncheckedEnqueue(Lit p, CRef from = CRef_Undef) {
    assert(value(p) == l_Undef);  // 确保文字未被赋值
    assigns[var(p)] = sign(p);    // 赋值
    vardata[var(p)] = std::move(mkVarData(from, decisionLevel()));  // 设置推理来源和决策层次
    trail.emplace_back(p);  // 将文字添加到trail中
}

// 选取分支文字（决策变量）
Lit pickBranchLit() {
    Var next = var_Undef;
    // 循环选择下一个未赋值的变量
    while (next == var_Undef or value(next) != l_Undef) {
        if (order_heap.empty()) {  // 如果决策队列为空，无法选择变量
            next = var_Undef;
            break;
        } else {
            auto p = *order_heap.rbegin();  // 选择活动性最高的变量
            next = p.second;
            order_heap.erase(p);  // 移除该变量
        }
    }
    return next == var_Undef ? lit_Undef : mkLit(next, polarity[next]);  // 返回选择的文字
}

// 子句学习和冲突分析
void analyze(CRef confl, std::vector<Lit> &out_learnt, int &out_btlevel) {
    int pathC = 0;  // 记录路径中的文字数量
    Lit p = lit_Undef;  // 当前文字
    int index = trail.size() - 1;  // 从trail的末尾开始分析
    out_learnt.emplace_back(mkLit(0, false));  // 初始化学习子句

    do {
        assert(confl != CRef_Undef);  // 确保冲突子句存在
        Clause &c = ca[confl];  // 获取冲突子句
        for (int j = (p == lit_Undef) ? 0 : 1; j < c.size(); j++) {
            Lit q = c[j];
            // 如果文字未被标记并且属于决策层次大于0的变量
            if (not seen[var(q)] and level(var(q)) > 0) {
                varBumpActivity(var(q));  // 增加该变量的活动性
                seen[var(q)] = 1;  // 标记该变量
                if (level(var(q)) >= decisionLevel()) {
                    pathC++;  // 如果该变量属于当前决策层次，增加路径计数
                } else {
                    out_learnt.emplace_back(q);  // 否则，将其添加到学习子句中
                }
            }
        }
        // 继续分析前一个文字，直到找到当前决策层次的文字
        while (not seen[var(trail[index--])])
            ;
        p = trail[index + 1];
        confl = reason(var(p));  // 获取前一个文字的推理来源
        seen[var(p)] = 0;  // 取消标记
        pathC--;  // 减少路径计数
    } while (pathC > 0);

    out_learnt[0] = ~p;  // 反转最先决策的文字，作为学习子句的第一个文字

    // 如果学习子句只有一个文字，将回溯层次设为0（单元子句）
    if (out_learnt.size() == 1) {
        out_btlevel = 0;
    } else {
        int max_i = 1;
        // 找到学习子句中决策层次最高的文字
        for (int i = 2; i < out_learnt.size(); i++) {
            if (level(var(out_learnt[i])) > level(var(out_learnt[max_i]))) {
                max_i = i;
            }
        }
        // 将该文字作为学习子句的第二个文字，更新回溯层次
        Lit p = out_learnt[max_i];
        out_learnt[max_i] = out_learnt[1];
        out_learnt[1] = p;
        out_btlevel = level(var(p));
    }

    // 清空标记
    for (int i = 0; i < out_learnt.size(); i++) {
        seen[var(out_learnt[i])] = false;
    }
}

 // 回溯到指定的决策层次，并取消更高层次的所有赋值
void cancelUntil(int level) {
    // 只有当前的决策层次大于目标层次时才进行回溯
    if (decisionLevel() > level) {
        // 遍历trail中的赋值，从当前决策层次到指定层次
        for (int c = trail.size() - 1; c >= trail_lim[level]; c--) {
            Var x = var(trail[c]);   // 获取变量
            assigns[x] = l_Undef;    // 取消该变量的赋值
            polarity[x] = sign(trail[c]);  // 恢复该变量的极性（正或负）
            // 将变量重新加入决策队列，以便后续可以重新进行决策
            order_heap.emplace(std::make_pair(activity[x], x));
        }
        qhead = trail_lim[level];  // 更新传播的起点
        // 清除trail和trail_lim中从当前层次以上的所有条目
        trail.erase(trail.end() - (trail.size() - trail_lim[level]), trail.end());
        trail_lim.erase(trail_lim.end() - (trail_lim.size() - level), trail_lim.end());
    }
}

// 传播所有已赋值的文字并检查冲突
CRef propagate() {
    CRef confl = CRef_Undef;  // 初始化冲突引用为未定义
    int num_props = 0;        // 记录传播次数

    // 遍历trail中的每个文字，进行传播
    while (qhead < trail.size()) {
        Lit p = trail[qhead++];  // 从trail中取出一个需要传播的文字
        std::vector<Watcher> &ws = watches[p.x];  // 获取该文字的监视列表
        std::vector<Watcher>::iterator i, j, end;
        num_props++;  // 增加传播计数

        // 遍历监视列表中的所有子句
        for (i = j = ws.begin(), end = i + ws.size(); i != end;) {
            // 获取阻挡文字，避免对子句进行不必要的检查
            Lit blocker = i->blocker;
            if (value(blocker) == l_True) {  // 如果阻挡文字为真，跳过当前子句
                *j++ = *i++;
                continue;
            }

            CRef cr = i->cref;  // 获取当前监视的子句引用
            Clause &c = ca[cr];  // 获取子句对象
            Lit false_lit = ~p;  // 获取当前传播文字的反转
            if (c[0] == false_lit) c[0] = c[1], c[1] = false_lit;  // 交换子句中的第一个和第二个文字
            assert(c[1] == false_lit);  // 确保第二个文字是false_lit
            i++;

            Lit first = c[0];  // 获取子句的第一个文字
            Watcher w = Watcher(cr, first);  // 创建新的监视器
            if (first != blocker && value(first) == l_True) {  // 如果第一个文字为真，继续下一步
                *j++ = w;
                continue;
            }

            // 尝试找到新的监视文字
            for (int k = 2; k < c.size(); k++) {
                if (value(c[k]) != l_False) {  // 如果找到一个非假文字
                    c[1] = c[k];  // 更新第二个监视文字
                    c[k] = false_lit;  // 将false_lit放回子句
                    watches[(~c[1]).x].emplace_back(w);  // 将新的监视器加入监视列表
                    goto NextClause;  // 跳过当前子句，处理下一个子句
                }
            }
            *j++ = w;  // 如果没有找到新的监视文字，继续当前监视器

            // 如果第一个文字的值为假，发生冲突
            if (value(first) == l_False) {
                confl = cr;  // 记录冲突的子句
                qhead = trail.size();  // 停止传播
                while (i < end) *j++ = *i++;  // 将剩余的监视器复制
            } else {
                uncheckedEnqueue(first, cr);  // 将第一个文字入队，继续传播
            }
        NextClause:;
        }
        int size = i - j;  // 计算需要移除的无效监视器数量
        ws.erase(ws.end() - size, ws.end());  // 移除无效的监视器
    }
    return confl;  // 返回冲突引用（如果有）
}

// Luby重启策略函数，返回luby序列值
static double luby(double y, int x) {
    int size, seq;
    // 找到包含索引'x'的有限子序列以及该子序列的大小
    for (size = 1, seq = 0; size < x + 1; seq++, size = 2 * size + 1)
        ;

    while (size - 1 != x) {
        size = (size - 1) >> 1;  // 缩减子序列
        seq--;
        x = x % size;  // 更新'x'为新子序列的相对索引
    }

    return pow(y, seq);  // 返回Luby序列的值
}

// 主要的搜索函数，包含传播、冲突处理、回溯、决策、和重启策略
lbool search(int nof_conflicts) {
    int backtrack_level;
    std::vector<Lit> learnt_clause;  // 用于存储学习子句
    learnt_clause.emplace_back(mkLit(-1, false));  // 初始化学习子句
    int conflictC = 0;  // 冲突计数

    while (true) {
        CRef confl = propagate();  // 进行传播并获取冲突子句

        if (confl != CRef_Undef) {  // 发生冲突
            conflictC++;  // 增加冲突计数
            if (decisionLevel() == 0) return l_False;  // 如果在决策层次0发生冲突，问题无解
            learnt_clause.clear();
            analyze(confl, learnt_clause, backtrack_level);  // 进行冲突分析，生成学习子句
            cancelUntil(backtrack_level);  // 回溯到适当的层次

            // 如果学习子句是单子句，将其直接加入到决策队列
            if (learnt_clause.size() == 1) {
                uncheckedEnqueue(learnt_clause[0]);
            } else {
                // 否则创建学习子句，并加入到子句集合
                CRef cr = allocClause(learnt_clause, true);
                attachClause(cr);  // 关联学习子句
                uncheckedEnqueue(learnt_clause[0], cr);  // 将学习子句的第一个文字入队
            }
            var_inc *= 1.05;  // 增加变量活动性的步长（变量衰减）
        } else {
            // 没有冲突的情况
            if ((nof_conflicts >= 0 and conflictC >= nof_conflicts)) {
                cancelUntil(0);  // 达到冲突限制后，回溯到最初层次
                return l_Undef;  // 返回未定义，表示需要重启
            }
            Lit next = pickBranchLit();  // 选择下一个决策变量

            if (next == lit_Undef) {
                return l_True;  // 如果没有更多变量可选，问题有解（SAT）
            }
            newDecisionLevel();  // 增加新的决策层次
            uncheckedEnqueue(next);  // 将决策变量入队
        }
    }
};


 public:
  // 当前的变量赋值。assigns[0] = 0 表示 X1 = True，assigns[1] = 1 表示 X2 = False
  std::vector<lbool> assigns;  
  // 存储求解的最终答案：0 表示可满足 (SATISFIABLE)，1 表示不可满足 (UNSATISFIABLE)，2 表示未知 (UNKNOWN)
  lbool answer;

  // 构造函数，初始化传播头（qhead）为 0
  Solver2() { qhead = 0; }

  // 解析DIMACS格式的CNF问题文件，读取变量和子句信息
  void parseDimacsProblem(std::string problem_name) {
    std::vector<Lit> lits;  // 存储临时读取的子句文字
    int vars = 0;           // 变量个数
    int clauses = 0;        // 子句个数
    std::string line;       // 当前读取的行
    std::ifstream ifs(problem_name, std::ios_base::in);  // 打开输入文件
    while (ifs.good()) {
      getline(ifs, line);  // 读取每一行
      if (line.size() > 0) {
        if (line[0] == 'p') {  // 如果是问题声明行
          sscanf(line.c_str(), "p cnf %d %d", &vars, &clauses);  // 解析变量和子句数量
        } else if (line[0] == 'c' or line[0] == 'p') {
          continue;  // 忽略注释行和问题声明行
        } else {
          // 读取子句信息并添加子句到求解器
          readClause(line, lits);
          if (lits.size() > 0) addClause_(lits);  // 如果子句不为空，添加到子句集
        }
      }
    }
    ifs.close();  // 关闭文件
  }

  // 主要的求解函数，采用Luby重启策略进行DPLL搜索，返回解的状态
  lbool solve() {
    model.clear();     // 清空模型
    conflict.clear();  // 清空冲突集
    lbool status = l_Undef;  // 初始化状态为未知
    answer = l_Undef;  // 初始化最终答案为未知
    var_inc = 1.01;    // 设置初始变量增量
    int curr_restarts = 0;  // 当前重启次数
    double restart_inc = 2;  // Luby序列的增加因子
    double restart_first = 100;  // 首次重启的基数
    while (status == l_Undef) {
      // 使用Luby策略计算重启基数，控制何时进行重启
      double rest_base = luby(restart_inc, curr_restarts);
      status = search(rest_base * restart_first);  // 进行搜索，尝试解决当前问题
      curr_restarts++;  // 重启计数增加
    }
    answer = status;  // 记录最终答案
    return status;    // 返回结果：SAT, UNSAT, 或 UNKNOWN
  }

  // 添加子句到求解器
  void addClause(std::vector<int> &clause) {
    std::vector<Lit> lits;  // 存储文字
    lits.resize(clause.size());  // 根据子句大小调整临时存储向量大小
    for (int i = 0; i < clause.size(); i++) {
      int var = abs(clause[i]) - 1;  // 将DIMACS索引转换为0索引的变量编号
      while (var >= nVars()) newVar();  // 如果变量还未分配，创建新变量
      // 根据子句中的正负号创建文字
      lits[i] = std::move((clause[i] > 0 ? mkLit(var, false) : mkLit(var, true)));
    }
    addClause_(lits);  // 添加处理好的子句到求解器
  }

  // 打印求解的结果
  void printAnswer() {
    if (answer == 0) {  // 如果是可满足的(SAT)
      std::cout << "SAT" << std::endl;
      for (int i = 0; i < assigns.size(); i++) {
        // 打印每个变量的赋值，如果是0表示True，否则为False
        if (assigns[i] == 0) {
          std::cout << (i + 1) << " ";  // 变量的正值表示True
        } else {
          std::cout << -(i + 1) << " ";  // 负值表示False
        }
      }
      std::cout << "0" << std::endl;  // 打印0表示结束
    } else {
      std::cout << "UNSAT" << std::endl;  // 如果是不可满足的(UNSAT)，输出UNSAT
    }
  }

};
}  // namespace cdcl
#endif  // cdcl_HPP
