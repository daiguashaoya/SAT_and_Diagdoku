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
#include<cstring>

using namespace std;
#define N 9
extern int random;
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
    Diagdoku diagdoku;
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
    diagdoku.generate_sudoku();

    // // 挖洞前的数独
    // cout << "完整的数独:\n";
    // print_sudoku(sudoku);

    // 挖洞操作，假设我们要挖20个洞
	cout<<"请输入想要挖的洞的数目\n";
	cin>>diagdoku.holes;
    diagdoku.remove_digits();
    diagdoku.copy_sudoku();
    // 挖洞后的数独
    cout << "\n生成的数独为:\n";
    diagdoku.print_modified_sudoku();

    // 生成数独 CNF
    toCnf();

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
            if(num!=sudoku[row-1][col-1])
            cout<<"填写错误！"<<endl;
            cout<<"继续填写则输入(i,j,num),不填写请输入0"<<endl;
            cin>>row;
        }
        break;
    case 4:
        if(newsudoku==sudoku)
        cout<<"填写正确了！"<<endl;
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