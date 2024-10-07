#include<new>
#include "solver.h"
int crossCNF::changeBool(int x)
//把负数翻译成一种比较大的正数的函数，以便其他函数处理负数文字 
{
	return (x>0)?x:(2*boolNum+1+x);
}
crossCNF::crossCNF(FILE *fp)//构造函数，可根据fp构造CNF对象 
{
	char get;
	
	//跳过注释：		
	while(1)
	{
		get=fgetc(fp);
		if(get=='c')while(fgetc(fp)!='\n'); // 跳过注释 
		else break;
	}
	
	//理论上，现在get=='p'
	//取元数据 
	fscanf(fp,"%*s%d%d",&boolNum,&clauseNum);
	
	remainClauseNum=clauseNum;
	
	
	//开辟数据空间 
	clauses = new crossNode[clauseNum+1];//头链表
	bools = new crossNode[2*boolNum+2];//节点块
	hypo = new BOOLVALUE[boolNum+1];//每个节点的假设
	sum = new int[clauseNum+1];//存储每个子句的剩余变元数
	
	
	int i,nowClause=1,temp;
	for(i=0;i<=clauseNum;i++)clauses[i].Bool=0;
	for(i=0;i<=boolNum;i++)hypo[i]=UNSURE;
	for(i=0;i<2*boolNum+2;i++)bools[i].Bool=0;
	for(i=0;i<=clauseNum;i++)sum[i]=0;
	//逐个存储
	while(nowClause<=clauseNum)
	{
		fscanf(fp,"%d",&temp);
		if(temp==0)nowClause++;
		else addNode(nowClause,temp);// 在此处添加十字链表结点 
	}
	fclose(fp);
	
	
	
	
	//初始化“单子句”栈 
	for(i=1;i<=clauseNum;i++)if(sum[i]==1)single.push(i);//逐个检查子句，将单子句序号纳入栈中
	
	//初始化“倾向” 
	tendency=new int[boolNum+1];//创建所有布尔变元的“倾向”数组
	int posi,nega;
	crossNode *p;
	for(i=1;i<=boolNum;i++)
	{
		posi=0;
		nega=0;
		// 记录正数出现多还是负数出现多 
		for(p=bools[i].down;p;p=p->down)posi++;
		for(p=bools[changeBool(-i)].down;p;p=p->down)nega++;
		if(posi>=nega)tendency[i]=1;
		else tendency[i]=-1;
	}
	return ;
}
crossCNF::crossCNF(const char * const filename)//构造函数，根据文件名构造CNF对象 
{
	FILE *fp=fopen(filename,"r");
	new (this)crossCNF(fp);
	fclose(fp);
}
crossCNF::~crossCNF()//析构函数 
{
	crossNode *p,*q;
	int i;
	//逐行删除 
	for(i=1;i<=clauseNum;i++)
	{
		p=clauses[i].right;
		while(p)
		{
			q=p->right;
			delete p;
			p=q;
		}
	}
	delete[] bools,clauses,hypo,sum,tendency;//删去手动分配的数组空间 
}
	
	
void crossCNF::print(FILE *fp=stdout)//依次打印目前cnf十字链表中的未被删除的内容
{
	crossNode *p;
	for(int i=1;i<=clauseNum;i++)
	{
		if(clauses[i].del)continue;
		fprintf(fp,"%d:",i);
		for(p=clauses[i].right;p!=NULL;p=p->right)
		{
			if(p->del)continue;
			fprintf(fp,"%d ",p->Bool);
		}
		fprintf(fp,"\n");
	}
}
void crossCNF::addNode(int Clause,int Bool)
{
	crossNode *p=new crossNode(),*p1,*p2;
	p->Bool=Bool;
	p->Clause=Clause;	
	Bool=changeBool(Bool);//正负数坐标转化为物理坐标 
	p1=&clauses[Clause];
	p2=&bools[Bool];
	
	while(p1->right!=NULL && changeBool(p1->right->Bool)<Bool)p1=p1->right;
	while(p2->down!=NULL && p2->down->Clause<Clause)p2=p2->down;
	if(p1->right)p1->right->left=p;
	p->right=p1->right;
	p1->right=p;
	p->left=p1;
	
	if(p2->down)p2->down->up=p;
	p->down=p2->down;
	p2->down=p;
	p->up=p2;
	
	sum[Clause]++;
}	

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
        int num = modified_sudoku[row][col];
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
void Diagdoku::print_sudoku(int (&now_sudoku)[N][N])
{
    std::cout << "=========================\n";  // 外边框上方
    for (int i = 0; i < 9; ++i) {
        if (i % 3 == 0 && i != 0) {
            std::cout << "-------------------------\n";  // 打印分隔线
        }
        std::cout << "| ";  // 左边框线
        for (int j = 0; j < 9; ++j) {
            if (j % 3 == 0 && j != 0) {
                std::cout << "| ";  // 打印竖分隔线
            }
	    	if(now_sudoku[i][j]==0)
			std::cout<<'.'<<' ';
			else
			std::cout << now_sudoku[i][j] << " ";
        }
        std::cout << "|\n";  // 右边框线和换行
    }
    std::cout << "=========================\n";  // 外边框下方
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