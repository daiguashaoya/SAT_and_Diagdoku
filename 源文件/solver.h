#include<cstdio>
#include<vector>
#include<bits/stdc++.h>
using namespace std;
#define MAXN 10000

enum BOOLVALUE {UNSURE, FALSE, TRUE};




struct crossNode
//ʮ������ڵ� 
{
	
	int Bool;//��ʾ�Լ��Ĳ�����Ԫ 
	int Clause;//��ʾ�Լ����Ӿ�
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
//intջ������ʹ������ջ 
//��ô�̵Ĵ���Ͳ�дcpp�ļ��� 
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
//ʮ������cnf 
{
	public:
		
		crossCNF(const char * const filename);//���캯�� 
		crossCNF(FILE *fp);//���캯�����ļ�ָ��棩 
		~crossCNF();//�������� 
		bool calculate(const char * const filename);//���� 
		void print(FILE *);//��ӡ 
		bool solve(const char* const filename,bool display);//�����
		bool solve(const char* const filename,const char* const filename0,bool display);
		int randomSolve(int Ans[]);
		int randomBelieve(int L,int level);
		int randomInnerSolve(int level); 
		
	private:
		int boolNum;//������Ԫ���� 
		int clauseNum;//�Ӿ������ԭ���ģ� 
		int remainClauseNum;//ɾ��һЩ֮��ʣ�µ��Ӿ���� 
		int *sum;//ÿ���Ӿ��ʣ�����ָ��� 
		int *tendency;//ÿ�����ֵġ����� 
		crossNode *bools;//��Ŵ�1��ʼ 
		crossNode *clauses;//��Ŵ�1��ʼ 
		BOOLVALUE *hypo;//����ʱʹ�õġ����衱 
		intStack single;//�浥�Ӿ��ջ 
		
		void addNode(int,int);//��ӽ�㣬����cnf����ʱҪ�� 
		int changeBool(int);//��[-boolNum,-1]U[1,boolNum]�����[1,2*boolNum]�ĺ��� 
		int innerSolve();//ʵ�ʽ� 
		int believe(int);//�����š������庬���뿴solver.cpp 
		void restore(int L,int startNum,crossNode *Head);//�ָ���believe�����޸Ĺ���cnf 
		
};

class Diagdoku
{
	private:
	static const int N = 9;
	int sudoku[N][N];//�洢�ڶ�֮ǰ����������
	int modified_sudoku[N][N];//�洢�ڶ�֮�������
	int newsudoku[N][N];//�洢�û���д֮�������
	
	public:
	int row,col,num;//�û���д���кš��кš��Լ�����
	int holes;//��Ҫ��ȥ�Ķ�����Ŀ
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
    v = v - 1;  // ת������ 0 ��ʼ������
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
    fprintf(fp, "p cnf 729 %d\n", 9558-holes );     //9477+81-holes �Խ��������������� 81-holes �������

    // ���Ӿ�
	for (int row = 0; row < N; ++row) {
    for (int col = 0; col < N; ++col) {
        int num = sudoku[row][col];
        if(num!=0)
		{
			fprintf(fp,"%d %d\n",var(row+1,col+1,num),0);
		}
    }
	}
    // ÿ��ÿ��ÿ���ÿ������ֻ�ܳ���һ��
    for (int x = 1; x <= 9; x++) {
        for (int y = 1; y <= 9; y++) {
            for (int z = 1; z <= 9; z++) {
                fprintf(fp, "%d ", (x - 1) * 81 + (y - 1) * 9 + z);
            }
            fprintf(fp, "0\n");
        }
    }
    // ��
    for (int y = 1; y <= 9; y++) {
        for (int z = 1; z <= 9; z++) {
            for (int x = 1; x <= 8; x++) {
                for (int i = x + 1; i <= 9; i++) {
                    fprintf(fp, "-%d -%d 0\n", (x - 1) * 81 + (y - 1) * 9 + z, (i - 1) * 81 + (y - 1) * 9 + z);
                }
            }
        }
    }
    // ��
    for (int x = 1; x <= 9; x++) {
        for (int z = 1; z <= 9; z++) {
            for (int y = 1; y <= 8; y++) {
                for (int i = y + 1; i <= 9; i++) {
                    fprintf(fp, "-%d -%d 0\n", (x - 1) * 81 + (y - 1) * 9 + z, (x - 1) * 81 + (i - 1) * 9 + z);
                }
            }
        }
    }
    // 3x3������
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
    // ���Խ���
    for (int z = 1; z <= 9; z++) {
        for (int i = 1; i <= 8; i++) {
            for (int j = i + 1; j <= 9; j++) {
                fprintf(fp, "-%d -%d 0\n", (i - 1) * 81 + (i - 1) * 9 + z, (j - 1) * 81 + (j - 1) * 9 + z);
            }
        }
    }
    // ���Խ���
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
	    // ����ԭʼ�������޸ĺ������
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            modified_sudoku[i][j]=sudoku[i][j];
        }
    }
    
    // ��������λ�õ��б�
    vector<int> positions;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            positions.push_back(i * N + j);
        }
    }

    // ����λ���б�
    random_shuffle(positions.begin(), positions.end());

    // �ڶ�
    for (int k = 0; k < holes; ++k) {
        int pos = positions[k];
        int row = pos / N;
        int col = pos % N;

        modified_sudoku[row][col] = 0; // ��ָ��λ����Ϊ0����ʾ�ڶ�
    }
}
bool Diagdoku::is_safe(int row, int col, int num) {
    // �����
    for (int x = 0; x < N; x++) {
        if (sudoku[row][x] == num) return false;
    }

    // �����
    for (int x = 0; x < N; x++) {
        if (sudoku[x][col] == num) return false;
    }

    // ���3x3�ӷ���
    int start_row = row - row % 3;
    int start_col = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (sudoku[i + start_row][j + start_col] == num) return false;
        }
    }

    // ���Խ���
    if (row == col) {  // ���Խ���
        for (int i = 0; i < N; i++) {
            if (sudoku[i][i] == num) return false;
        }
    }
    if (row + col == N - 1) {  // ���Խ���
        for (int i = 0; i < N; i++) {
            if (sudoku[i][N - 1 - i] == num) return false;
        }
    }

    return true;
}
bool Diagdoku::fill_sudoku(int row, int col) {
    // ����Ѿ�����������true
    if (row == N - 1 && col == N) return true;

    // �������������������һ��
    if (col == N) {
        row++;
        col = 0;
    }

    // �����ǰλ���Ѿ���ֵ���ݹ���һ��λ��
    if (sudoku[row][col] != 0) return fill_sudoku(row, col + 1);

    // �������1��9������
    vector<int> numbers(N);
    iota(numbers.begin(), numbers.end(), 1);  // ����1��9������
    random_shuffle(numbers.begin(), numbers.end());  // �����������˳��

    for (int num : numbers) {
        if (is_safe(row, col, num)) {
            sudoku[row][col] = num;

            // �ݹ������һ��λ��
            if (fill_sudoku(row, col + 1)) return true;

            // ���ݣ�������ʧ�ܣ��ָ�0
            sudoku[row][col] = 0;
        }
    }

    return false;
}

void Diagdoku::generate_sudoku() {
    fill_sudoku(0, 0);
}