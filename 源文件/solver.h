#include<cstdio>
#include<vector>
#include<bits/stdc++.h>
using namespace std;
#define MAXN 10000

enum BOOLVALUE {UNSURE, FALSE, TRUE};




struct crossNode
//十字链表节点 
{
	
	int Bool;//表示自己的布尔变元 
	int Clause;//表示自己的子句
	bool del;
	crossNode *up,*down,*right,*left,*next;
	crossNode()
	{
		Bool=0;
		Clause=0;
		up=NULL;
		down=NULL;
		right=NULL;
		left=NULL;
		next=NULL;
		del=false;
	}

};

struct intStack
//int栈，这里使用数组栈 
//这么短的代码就不写cpp文件了 
{
	int S[MAXN];
	int i=0;
	void push(int value)
	{
		S[i++]=value;
	}
	
	int pop()
	{
		//assert(i>0);
		return S[--i];
	}

	bool empty()
	{
		return i==0;
	}

};

class crossCNF
//十字链表cnf 
{
	public:
		
		crossCNF(const char * const filename);//构造函数 
		crossCNF(FILE *fp);//构造函数（文件指针版） 
		~crossCNF();//析构函数 
		bool calculate(const char * const filename);//检验 
		void print(FILE *);//打印 
		bool solve(const char* const filename,bool display);//名义解
		bool solve(const char* const filename,const char* const filename0,bool display);
		int randomSolve(int Ans[]);
		int randomBelieve(int L,int level);
		int randomInnerSolve(int level); 
		
	private:
		int boolNum;//布尔变元个数 
		int clauseNum;//子句个数（原来的） 
		int remainClauseNum;//删了一些之后剩下的子句个数 
		int *sum;//每个子句的剩余文字个数 
		int *tendency;//每个文字的“倾向” 
		crossNode *bools;//编号从1开始 
		crossNode *clauses;//编号从1开始 
		BOOLVALUE *hypo;//搜索时使用的“假设” 
		intStack single;//存单子句的栈 
		
		void addNode(int,int);//添加结点，生成cnf对象时要用 
		int changeBool(int);//把[-boolNum,-1]U[1,boolNum]翻译成[1,2*boolNum]的函数 
		int innerSolve();//实际解 
		int believe(int);//“相信”，具体含义请看solver.cpp 
		void restore(int L,int startNum,crossNode *Head);//恢复被believe函数修改过的cnf 
		
};

class Diagdoku
{
	private:
	static const int N = 9;
	int sudoku[N][N];//存储挖洞之前的完整数独
	int modified_sudoku[N][N];//存储挖洞之后的数独
	int newsudoku[N][N];//存储用户填写之后的数独
	
	public:
	int row,col,num;//用户填写的行号、列号、以及数字
	int holes;//需要挖去的洞的数目
	void generate_sudoku();
	bool fill_sudoku(int row, int col);
	bool is_safe( int row, int col, int num);
	void remove_digits();
	void print_sudoku();
	void print_modified_sudoku();
	void toCnf();
	int var(int i, int j, int k);
	void inverse_var(int v, int &i, int &j, int &k);
	void copy_sudoku();
};

void Diagdoku::copy_sudoku()
{	
	for(int i=0;i<N;i++)
	{
		for(int j=0;j<N;j++)
		newsudoku[i][j]=modified_sudoku[i][j];
	}
}
void Diagdoku::inverse_var(int v, int &i, int &j, int &k) {
    v = v - 1;  // 转换到从 0 开始的索引
    i = v / (N * N) + 1;
    j = (v % (N * N)) / N + 1;  
    k = v % N + 1;
}

int Diagdoku::var(int i, int j, int k) {
    return (i - 1) * N * N + (j - 1) * N + k;
}

void Diagdoku::toCnf()
{
	    FILE *fp=fopen("Diagdoku.cnf","w");
	
    if (!fp) {
        printf("Cannot open file for writing!\n");
        return;
    }
	fprintf(fp,"c\nc\n");
    fprintf(fp, "p cnf 729 %d\n", 9558-holes );     //9477+81-holes 对角线数独限制条件 81-holes 填入的数

    // 单子句
	for (int row = 0; row < N; ++row) {
    for (int col = 0; col < N; ++col) {
        int num = sudoku[row][col];
        if(num!=0)
		{
			fprintf(fp,"%d %d\n",var(row+1,col+1,num),0);
		}
    }
	}
    // 每行每列每块的每个数字只能出现一次
    for (int x = 1; x <= 9; x++) {
        for (int y = 1; y <= 9; y++) {
            for (int z = 1; z <= 9; z++) {
                fprintf(fp, "%d ", (x - 1) * 81 + (y - 1) * 9 + z);
            }
            fprintf(fp, "0\n");
        }
    }
    // 行
    for (int y = 1; y <= 9; y++) {
        for (int z = 1; z <= 9; z++) {
            for (int x = 1; x <= 8; x++) {
                for (int i = x + 1; i <= 9; i++) {
                    fprintf(fp, "-%d -%d 0\n", (x - 1) * 81 + (y - 1) * 9 + z, (i - 1) * 81 + (y - 1) * 9 + z);
                }
            }
        }
    }
    // 列
    for (int x = 1; x <= 9; x++) {
        for (int z = 1; z <= 9; z++) {
            for (int y = 1; y <= 8; y++) {
                for (int i = y + 1; i <= 9; i++) {
                    fprintf(fp, "-%d -%d 0\n", (x - 1) * 81 + (y - 1) * 9 + z, (x - 1) * 81 + (i - 1) * 9 + z);
                }
            }
        }
    }
    // 3x3子网格
    for (int z = 1; z <= 9; z++) {
        for (int i = 0; i <= 2; i++) {
            for (int j = 0; j <= 2; j++) {
                for (int x = 1; x <= 3; x++) {
                    for (int y = 1; y <= 3; y++) {
                        for (int k = y + 1; k <= 3; k++) {
                            fprintf(fp, "-%d -%d 0\n", (3 * i + x - 1) * 81 + (3 * j + y - 1) * 9 + z, (3 * i + x - 1) * 81 + (3 * j + k - 1) * 9 + z);
						}
                    }
                }
            }
        }
    }
    for (int z = 1; z <= 9; z++) {
        for (int i = 0; i <= 2; i++) {
            for (int j = 0; j <= 2; j++) {
                for (int x = 1; x <= 3; x++) {
                    for (int y = 1; y <= 3; y++) {
                        for (int k = x + 1; k <= 3; k++) {
                            for (int l = 1; l <= 3; l++) {
                                fprintf(fp, "-%d -%d 0\n", (3 * i + x - 1) * 81 + (3 * j + y - 1) * 9 + z, (3 * i + k - 1) * 81 + (3 * j + l - 1) * 9 + z);
							}
                        }
                    }
                }
            }
        }
    }
    // 主对角线
    for (int z = 1; z <= 9; z++) {
        for (int i = 1; i <= 8; i++) {
            for (int j = i + 1; j <= 9; j++) {
                fprintf(fp, "-%d -%d 0\n", (i - 1) * 81 + (i - 1) * 9 + z, (j - 1) * 81 + (j - 1) * 9 + z);
            }
        }
    }
    // 副对角线
    for (int z = 1; z <= 9; z++) {
        for (int i = 1; i <= 8; i++) {
            for (int j = i + 1; j <= 9; j++) {
                fprintf(fp, "-%d -%d 0\n", (i - 1) * 81 + (9 - i) * 9 + z, (j - 1) * 81 + (9 - j) * 9 + z);
            }
        }
    }
    fclose(fp);
}
void Diagdoku::print_sudoku()
{
	    for (const auto& row : sudoku) {
        for (int num : row) {
            cout << (num == 0 ? "." : to_string(num)) << " ";
        }
        cout  << endl;
    }
}
void Diagdoku::print_modified_sudoku()
{
	    for (const auto& row : modified_sudoku) {
        for (int num : row) {
            cout << (num == 0 ? "." : to_string(num)) << " ";
        }
        cout  << endl;
    }
}
void Diagdoku::remove_digits()
{
	    // 复制原始数独到修改后的数独
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            modified_sudoku[i][j]=sudoku[i][j];
        }
    }
    
    // 生成所有位置的列表
    vector<int> positions;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            positions.push_back(i * N + j);
        }
    }

    // 打乱位置列表
    random_shuffle(positions.begin(), positions.end());

    // 挖洞
    for (int k = 0; k < holes; ++k) {
        int pos = positions[k];
        int row = pos / N;
        int col = pos % N;

        modified_sudoku[row][col] = 0; // 将指定位置设为0，表示挖洞
    }
}
bool Diagdoku::is_safe(int row, int col, int num) {
    // 检查行
    for (int x = 0; x < N; x++) {
        if (sudoku[row][x] == num) return false;
    }

    // 检查列
    for (int x = 0; x < N; x++) {
        if (sudoku[x][col] == num) return false;
    }

    // 检查3x3子方格
    int start_row = row - row % 3;
    int start_col = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (sudoku[i + start_row][j + start_col] == num) return false;
        }
    }

    // 检查对角线
    if (row == col) {  // 主对角线
        for (int i = 0; i < N; i++) {
            if (sudoku[i][i] == num) return false;
        }
    }
    if (row + col == N - 1) {  // 副对角线
        for (int i = 0; i < N; i++) {
            if (sudoku[i][N - 1 - i] == num) return false;
        }
    }

    return true;
}
bool Diagdoku::fill_sudoku(int row, int col) {
    // 如果已经填满，返回true
    if (row == N - 1 && col == N) return true;

    // 如果列数超出，进入下一行
    if (col == N) {
        row++;
        col = 0;
    }

    // 如果当前位置已经有值，递归下一个位置
    if (sudoku[row][col] != 0) return fill_sudoku(row, col + 1);

    // 尝试填充1到9的数字
    vector<int> numbers(N);
    iota(numbers.begin(), numbers.end(), 1);  // 生成1到9的数字
    random_shuffle(numbers.begin(), numbers.end());  // 随机打乱数字顺序

    for (int num : numbers) {
        if (is_safe(row, col, num)) {
            sudoku[row][col] = num;

            // 递归填充下一个位置
            if (fill_sudoku(row, col + 1)) return true;

            // 回溯，如果填充失败，恢复0
            sudoku[row][col] = 0;
        }
    }

    return false;
}

void Diagdoku::generate_sudoku() {
    fill_sudoku(0, 0);
}