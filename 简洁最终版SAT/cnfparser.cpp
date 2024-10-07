#include<new>
#include "solver.h"
int crossCNF::changeBool(int x)
//�Ѹ��������һ�ֱȽϴ�������ĺ������Ա������������������� 
{
	return (x>0)?x:(2*boolNum+1+x);
}
crossCNF::crossCNF(FILE *fp)//���캯�����ɸ���fp����CNF���� 
{
	char get;
	
	//����ע�ͣ�		
	while(1)
	{
		get=fgetc(fp);
		if(get=='c')while(fgetc(fp)!='\n'); // ����ע�� 
		else break;
	}
	
	//�����ϣ�����get=='p'
	//ȡԪ���� 
	fscanf(fp,"%*s%d%d",&boolNum,&clauseNum);
	
	remainClauseNum=clauseNum;
	
	
	//�������ݿռ� 
	clauses = new crossNode[clauseNum+1];//ͷ����
	bools = new crossNode[2*boolNum+2];//�ڵ��
	hypo = new BOOLVALUE[boolNum+1];//ÿ���ڵ�ļ���
	sum = new int[clauseNum+1];//�洢ÿ���Ӿ��ʣ���Ԫ��
	
	
	int i,nowClause=1,temp;
	for(i=0;i<=clauseNum;i++)clauses[i].Bool=0;
	for(i=0;i<=boolNum;i++)hypo[i]=UNSURE;
	for(i=0;i<2*boolNum+2;i++)bools[i].Bool=0;
	for(i=0;i<=clauseNum;i++)sum[i]=0;
	//����洢
	while(nowClause<=clauseNum)
	{
		fscanf(fp,"%d",&temp);
		if(temp==0)nowClause++;
		else addNode(nowClause,temp);// �ڴ˴����ʮ�������� 
	}
	fclose(fp);
	
	
	
	
	//��ʼ�������Ӿ䡱ջ 
	for(i=1;i<=clauseNum;i++)if(sum[i]==1)single.push(i);//�������Ӿ䣬�����Ӿ��������ջ��
	
	//��ʼ�������� 
	tendency=new int[boolNum+1];//�������в�����Ԫ�ġ���������
	int posi,nega;
	crossNode *p;
	for(i=1;i<=boolNum;i++)
	{
		posi=0;
		nega=0;
		// ��¼�������ֶ໹�Ǹ������ֶ� 
		for(p=bools[i].down;p;p=p->down)posi++;
		for(p=bools[changeBool(-i)].down;p;p=p->down)nega++;
		if(posi>=nega)tendency[i]=1;
		else tendency[i]=-1;
	}
	return ;
}
crossCNF::crossCNF(const char * const filename)//���캯���������ļ�������CNF���� 
{
	FILE *fp=fopen(filename,"r");
	new (this)crossCNF(fp);
	fclose(fp);
}
crossCNF::~crossCNF()//�������� 
{
	crossNode *p,*q;
	int i;
	//����ɾ�� 
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
	delete[] bools,clauses,hypo,sum,tendency;//ɾȥ�ֶ����������ռ� 
}
	
	
void crossCNF::print(FILE *fp=stdout)//���δ�ӡĿǰcnfʮ�������е�δ��ɾ��������
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
	Bool=changeBool(Bool);//����������ת��Ϊ�������� 
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
        int num = modified_sudoku[row][col];
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
void Diagdoku::print_sudoku(int (&now_sudoku)[N][N])
{
    std::cout << "=========================\n";  // ��߿��Ϸ�
    for (int i = 0; i < 9; ++i) {
        if (i % 3 == 0 && i != 0) {
            std::cout << "-------------------------\n";  // ��ӡ�ָ���
        }
        std::cout << "| ";  // ��߿���
        for (int j = 0; j < 9; ++j) {
            if (j % 3 == 0 && j != 0) {
                std::cout << "| ";  // ��ӡ���ָ���
            }
	    	if(now_sudoku[i][j]==0)
			std::cout<<'.'<<' ';
			else
			std::cout << now_sudoku[i][j] << " ";
        }
        std::cout << "|\n";  // �ұ߿��ߺͻ���
    }
    std::cout << "=========================\n";  // ��߿��·�
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