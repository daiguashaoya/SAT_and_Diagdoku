#include<iostream>
#include<stdexcept>
#include<cstdio>
#include<cstdlib>
#include "solver.h"
#include <ctime> 
#include <vector>
#include <algorithm>
#include <time.h>
#include <numeric>
#include <cstring>
using namespace std;
extern int random;
#define N 9  // 9x9 数独
int inputOrder(int stt,int end,const char* const text=NULL)
//这个函数用来输入选项，选择范围是[stt,end]
//将此单独设置成函数，可以避免重复的检查输入是否正确
//以返回值的形式输出结果 
//会把字符串text输出在屏幕上，以提示用户输入 
{
	
	if(end<stt)throw runtime_error("in inputOrder():end<stt\n");//遇到输入问题时报错。如真报错，肯定是代码问题而非输入问题 
	int i,ans=-1;
	/*
	i:循环变量
	ans:待返回的值 
	
	*/ 
	char s[1000];//输入的一行，应该没有人会一行打1000个字符吧 
	bool ok=false;//用于标记输入是否合法。 
	if(text)printf("%s",text);//输出提示文字 
	while(1)//主循环。如输入合法，返回输入，否则再次循环 
	{
		fgets(s,sizeof(s),stdin);//“安全地”从屏幕上读入一行字符串 
		ok=true;
		
		for(i=0;s[i]!='\n';i++)//检查输入是否含有非数字字符 
		{
			if(s[i]<'0' || s[i]>'9')
			{
				printf("输入含有非数字字符(%c,%d)，请重试\n",s[i],s[i]);
				ok=false;
				break;//跳出for循环 
			}
		}
		s[i]='\0';//标记字符串的末尾 
		if(ok==false)continue;
		if(strlen(s)==0)continue;//这是为了应对用户输入一个空行的情况 

		ans=atoi(s);// atoi:把字符串形式的整数转化为int 
		if(ans<stt || ans>end)// 判断是否超出范围 
		{
			printf("输入超出范围，请重试\n");
			ok=false;
			continue;
		} 
		return ans;//如未超出范围，返回
	}
	
	
	
}

void useDPLL()
//此函数用于演示dpll模块
{
	srand(clock());
	
	char filename[500];//cnf文件名 
	char filename2[500];//res文件名 
	crossCNF *cnf=NULL;//待定的cnf对象 
	int choice;//选项 
	FILE *fp=NULL;//待定文件指针 
	bool solved=false;//表示当前cnf是否被解过。被解过的cnf对象数据结构会被破坏，不能再解，再解需要重新从文件中读取 
	while(1)//主循环 
	{
		system("cls");//清屏 
		choice=inputOrder(0,4,
		"||^^^^^^^^^^^^^^^^^^^^^^^||\n"
		"|| 选择操作：            ||\n"
		"|| （1）.cnf的读取与存储 || \n"
		"|| （2）.解已指定的cnf   ||\n"
		"|| （3）.检验结果        ||\n"
		"||  (0) .退出            ||\n"
		"||^^^^^^^^^^^^^^^^^^^^^^^||\n");
		
		switch(choice)
		{
			case 0:{//退出 
				if(cnf!=NULL)delete cnf;//释放cnf 
				return;
			}
			case 1:{//指定cnf 
				//除了指定cnf文件名之外，这个块还需要重新初始化其他内容
				if(cnf!=NULL)
				{
					printf("cnf已经定义，正在释放旧cnf\n");
					delete cnf;//释放cnf 
					cnf=NULL;
					fclose(fp);
					fp=NULL;
				}
				
				
				while(fp==NULL)
				{
					printf("指定需要求解的cnf文件名：\n");
					scanf("%s",filename);
					
					fp=fopen(filename,"r");
					if(fp==NULL)printf("打开文件失败！重试！\n");
				}
				//现在已经打开文件成功
				cnf=new crossCNF(fp);//重新初始化
				solved=false;
				//然后还要重新赋值filename2
				strcpy(filename2,filename);
				int i=0;
				while(filename2[i]!='.')i++;
				filename2[i+1]='r';
				filename2[i+2]='e';
				filename2[i+3]='s';
				filename2[i+4]='\0';
				
				printf("输出文件被指定为%s\n",filename2);
				break;
			}
			
			case 2:{//求解已指定的cnf 
				if(cnf==NULL)
				{
					printf("还未指定cnf!\n");
					break;
				}
				if(solved)
				{
					printf("这个cnf已经被解过， 数据结构被破坏，请重新初始化！\n");
					break;
				}
				
				cnf->solve(filename2,filename,true);
				solved=true;
				printf("求解结果已经被写入%s\n",filename2);
				break;
			}
			
			case 3:{//验证并显示  
				if(cnf==NULL)
				{
					printf("还未指定cnf!\n");
					break;
				}
				if(solved==false)
				{
					printf("这个cnf还未被求解过！\n");
					break;
				}				 
				cnf->calculate(filename2);
				break;
			}
			
			
		}
		system("pause");
	}
	
	
	
}


// 将 (i, j, k) 转换为唯一的整数变量
int var(int i, int j, int k) {
    return (i - 1) * N * N + (j - 1) * N + k;
}
void inverse_var(int v, int &i, int &j, int &k) {
    v = v - 1;  // 转换到从 0 开始的索引
    i = v / (N * N) + 1;
    j = (v % (N * N)) / N + 1;  
    k = v % N + 1;
}
void toCnf(vector<vector<int>>& sudoku, int holes) {
    FILE *fp=fopen("Diagdoku.cnf","w");
	
    if (!fp) {
        printf("Cannot open file for writing!\n");
        return;
    }
	fprintf(fp,"c\nc\n");
    fprintf(fp, "p cnf 729 %d\n", 9558-holes );     //9477+81-holes 对角线数独限制条件 81-holes 填入的数

    // 单子句
	for (int row = 0; row < sudoku.size(); ++row) {
    for (int col = 0; col < sudoku[row].size(); ++col) {
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



// 打印数独
void print_sudoku(const vector<vector<int>>& sudoku) {
    for (const auto& row : sudoku) {
        for (int num : row) {
            cout << (num == 0 ? "." : to_string(num)) << " ";
        }
        cout  << endl;
    }
}

// 检查数字num是否可以放在(row, col)位置上
bool is_safe(const vector<vector<int>>& sudoku, int row, int col, int num) {
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

// 使用递归回溯算法填充数独
bool fill_sudoku(vector<vector<int>>& sudoku, int row, int col) {
    // 如果已经填满，返回true
    if (row == N - 1 && col == N) return true;

    // 如果列数超出，进入下一行
    if (col == N) {
        row++;
        col = 0;
    }

    // 如果当前位置已经有值，递归下一个位置
    if (sudoku[row][col] != 0) return fill_sudoku(sudoku, row, col + 1);

    // 尝试填充1到9的数字
    vector<int> numbers(N);
    iota(numbers.begin(), numbers.end(), 1);  // 生成1到9的数字
    random_shuffle(numbers.begin(), numbers.end());  // 随机打乱数字顺序

    for (int num : numbers) {
        if (is_safe(sudoku, row, col, num)) {
            sudoku[row][col] = num;

            // 递归填充下一个位置
            if (fill_sudoku(sudoku, row, col + 1)) return true;

            // 回溯，如果填充失败，恢复0
            sudoku[row][col] = 0;
        }
    }

    return false;
}

// 生成一个完整的9x9对角线数独
void generate_sudoku(vector<vector<int>>& sudoku) {
    fill_sudoku(sudoku, 0, 0);
}

// 挖洞法
void remove_digits(const vector<vector<int>>& original_sudoku, vector<vector<int>>& modified_sudoku, int holes) {
    // 复制原始数独到修改后的数独
    modified_sudoku = original_sudoku;
    
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
void useDiagdoku()
//此函数用于使用蜂窝数独模块
{
	int choice;
	choice=inputOrder(0,4,
			"||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
			"|| 选择功能：                ||\n"
			"|| 1.生成数独                ||\n"
			"|| 2.运用SAT求解器求解数独    ||\n"
            "|| 3.进行数独填充             ||\n"
            "|| 4.检验答案                 ||\n"
			"|| 0.退出                    ||\n"
			"||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
			);
	// 初始化一个9x9空的数独
    vector<vector<int>> sudoku(N, vector<int>(N, 0));
    vector<vector<int>> modified_sudoku(N, vector<int>(N, 0));
    vector<vector<int>> newsudoku;
    int row,col,num;
	crossCNF *cnf=NULL;//待定的cnf对象
	FILE *file=fopen("Diagdoku.cnf","r");
	FILE *file2=NULL;
	char line1[10],s[2];
    int temp1,i,j,k;
    int ans[N][N],cnt=1;
	while(choice!=0)
    {
        system("cls");
        switch (choice)
	{
	case 1:
		srand(time(0)); // 使用当前时间作为随机数种子
    // 生成对角线数独
    generate_sudoku(sudoku);

    // // 挖洞前的数独
    // cout << "完整的数独:\n";
    // print_sudoku(sudoku);

    // 挖洞操作，假设我们要挖20个洞
	int holes;
	cout<<"请输入想要挖的洞的数目\n";
	cin>>holes;
    remove_digits(sudoku, modified_sudoku, holes);
    newsudoku=modified_sudoku;
    // 挖洞后的数独
    cout << "\n生成的数独为:\n";
    print_sudoku(modified_sudoku);

    // 生成数独 CNF
    toCnf(modified_sudoku,holes);

	printf("已生成相应的cnf文件！\n");
	system("pause");
		break;
	
	case 2:
		cnf=new crossCNF(file);//重新初始化
		cnf->solve("Diagdoku.res","Diagdoku.cnf",true);
        file2=fopen("Diagdoku.res","r");
    // 读取第一行
        fgets(line1, sizeof(line1), file2);
    // 读取第二行头两个字符
        fgets(s,sizeof(s),file2);
        while(cnt<=81)
        {   
            fscanf(file2,"%d ",&temp1);
            if(temp1>0)
            {
                inverse_var(temp1,i,j,k);
                ans[i-1][j-1]=k;
                cnt++;
            }
        }   
    // 关闭文件
    	fclose(file2);
        cout << "标准答案:\n";
        print_sudoku(sudoku);
        printf("SAT求解答案：\n");
        for(int i=0;i<N;i++)
        {
            for(int j=0;j<N;j++)
            printf("%d ",ans[i][j]);
            printf("\n");
        }
		printf("程序结束");
		break;
    case 3:
        newsudoku=modified_sudoku;
        cout<<"当前数独如下"<<endl;
        print_sudoku(newsudoku);
        cout<<"请输入你想填入数独的位置（i，j），以及填入的数字num（只需输1个0退出填写）"<<endl;
        cin>>row;
        while(row!=0)
        {
            cin>>col>>num;
            newsudoku[row-1][col-1]=num;
            cout<<"当前数独如下"<<endl;
            print_sudoku(newsudoku);
            cout<<"继续填写则输入(i,j,num),不填写请输入0"<<endl;
            cin>>row;
        }
        break;
    case 4:
        if(newsudoku==sudoku)
        cout<<"你他娘的真是个天才！"<<endl;
        else
        cout<<"填写不对哦，请再试一次吧！";
        cout<<"你填写的结果"<<endl;
        print_sudoku(newsudoku);
        cout<<"标准答案"<<endl;
        print_sudoku(sudoku);
        break;

	default:
		break;
	}
    choice=inputOrder(0,4,
			"||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
			"|| 选择功能：                ||\n"
			"|| 1.生成数独                ||\n"
			"|| 2.运用SAT求解器求解数独    ||\n"
            "|| 3.进行数独填充             ||\n"
            "|| 4.检验答案                 ||\n"
			"|| 0.退出                    ||\n"
			"||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
			);
    }
}




int main()
{
	int choice;
	while(1)
	{
		system("cls");
		choice=inputOrder(0,2,
			"||^^^^^^^^^^^^^^^^^^^^^^^||\n"
			"|| 选择功能：            ||\n"
			"|| 1.SAT求解  模块       ||\n"
			"|| 2.Diagdoku 模块       ||\n"
			"|| 0.退出                ||\n"
			"||^^^^^^^^^^^^^^^^^^^^^^^||\n"
			);
	
		switch(choice)
		{
			case 0:{
				return 0;//此时退出就是结束整个程序 
			}
			case 1:{
				useDPLL();
				break;
			}
			
			case 2:{
				useDiagdoku();
				break;
			}
		}
		
	}
	
	
}
