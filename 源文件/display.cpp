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
//���������������ѡ�ѡ��Χ��[stt,end]
//���˵������óɺ��������Ա����ظ��ļ�������Ƿ���ȷ
//�Է���ֵ����ʽ������ 
//����ַ���text�������Ļ�ϣ�����ʾ�û����� 
{
	
	if(end<stt)throw runtime_error("in inputOrder():end<stt\n");//������������ʱ�������汨���϶��Ǵ������������������ 
	int i,ans=-1;
	/*
	i:ѭ������
	ans:�����ص�ֵ 
	
	*/ 
	char s[1000];//�����һ�У�Ӧ��û���˻�һ�д�1000���ַ��� 
	bool ok=false;//���ڱ�������Ƿ�Ϸ��� 
	if(text)printf("%s",text);//�����ʾ���� 
	while(1)//��ѭ����������Ϸ����������룬�����ٴ�ѭ�� 
	{
		fgets(s,sizeof(s),stdin);//����ȫ�ء�����Ļ�϶���һ���ַ��� 
		ok=true;
		
		for(i=0;s[i]!='\n';i++)//��������Ƿ��з������ַ� 
		{
			if(s[i]<'0' || s[i]>'9')
			{
				printf("���뺬�з������ַ�(%c,%d)��������\n",s[i],s[i]);
				ok=false;
				break;//����forѭ�� 
			}
		}
		s[i]='\0';//����ַ�����ĩβ 
		if(ok==false)continue;
		if(strlen(s)==0)continue;//����Ϊ��Ӧ���û�����һ�����е���� 

		ans=atoi(s);// atoi:���ַ�����ʽ������ת��Ϊint 
		if(ans<stt || ans>end)// �ж��Ƿ񳬳���Χ 
		{
			printf("���볬����Χ��������\n");
			ok=false;
			continue;
		} 
		return ans;//��δ������Χ������
	}
	
	
	
}

void useDPLL()
//�˺���������ʾdpllģ��
{
	srand(clock());
	
	char filename[500];//cnf�ļ��� 
	char filename2[500];//res�ļ��� 
	crossCNF *cnf=NULL;//������cnf���� 
	int choice;//ѡ�� 
	FILE *fp=NULL;//�����ļ�ָ�� 
	bool solved=false;//��ʾ��ǰcnf�Ƿ񱻽�����������cnf�������ݽṹ�ᱻ�ƻ��������ٽ⣬�ٽ���Ҫ���´��ļ��ж�ȡ 
	while(1)//��ѭ�� 
	{
		system("cls");//���� 
		choice=inputOrder(0,4,
		"||^^^^^^^^^^^^^^^^^^^^^^^||\n"
		"|| ѡ�������            ||\n"
		"|| ��1��.cnf�Ķ�ȡ��洢 || \n"
		"|| ��2��.����ָ����cnf   ||\n"
		"|| ��3��.������        ||\n"
		"||  (0) .�˳�            ||\n"
		"||^^^^^^^^^^^^^^^^^^^^^^^||\n");
		
		switch(choice)
		{
			case 0:{//�˳� 
				if(cnf!=NULL)delete cnf;//�ͷ�cnf 
				return;
			}
			case 1:{//ָ��cnf 
				//����ָ��cnf�ļ���֮�⣬����黹��Ҫ���³�ʼ����������
				if(cnf!=NULL)
				{
					printf("cnf�Ѿ����壬�����ͷž�cnf\n");
					delete cnf;//�ͷ�cnf 
					cnf=NULL;
					fclose(fp);
					fp=NULL;
				}
				
				
				while(fp==NULL)
				{
					printf("ָ����Ҫ����cnf�ļ�����\n");
					scanf("%s",filename);
					
					fp=fopen(filename,"r");
					if(fp==NULL)printf("���ļ�ʧ�ܣ����ԣ�\n");
				}
				//�����Ѿ����ļ��ɹ�
				cnf=new crossCNF(fp);//���³�ʼ��
				solved=false;
				//Ȼ��Ҫ���¸�ֵfilename2
				strcpy(filename2,filename);
				int i=0;
				while(filename2[i]!='.')i++;
				filename2[i+1]='r';
				filename2[i+2]='e';
				filename2[i+3]='s';
				filename2[i+4]='\0';
				
				printf("����ļ���ָ��Ϊ%s\n",filename2);
				break;
			}
			
			case 2:{//�����ָ����cnf 
				if(cnf==NULL)
				{
					printf("��δָ��cnf!\n");
					break;
				}
				if(solved)
				{
					printf("���cnf�Ѿ�������� ���ݽṹ���ƻ��������³�ʼ����\n");
					break;
				}
				
				cnf->solve(filename2,filename,true);
				solved=true;
				printf("������Ѿ���д��%s\n",filename2);
				break;
			}
			
			case 3:{//��֤����ʾ  
				if(cnf==NULL)
				{
					printf("��δָ��cnf!\n");
					break;
				}
				if(solved==false)
				{
					printf("���cnf��δ��������\n");
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
//�˺�������ʹ�÷�������ģ��
{
	int choice;
	choice=inputOrder(0,4,
			"||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
			"|| ѡ���ܣ�                ||\n"
			"|| 1.��������                ||\n"
			"|| 2.����SAT������������    ||\n"
            "|| 3.�����������             ||\n"
            "|| 4.�����                 ||\n"
			"|| 0.�˳�                    ||\n"
			"||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
			);
	// ��ʼ��һ��9x9�յ�����
    Diagdoku diagdoku;
	crossCNF *cnf=NULL;//������cnf����
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
		srand(time(0)); // ʹ�õ�ǰʱ����Ϊ���������
    // ���ɶԽ�������
    diagdoku.generate_sudoku();

    // // �ڶ�ǰ������
    // cout << "����������:\n";
    // print_sudoku(sudoku);

    // �ڶ���������������Ҫ��20����
	cout<<"��������Ҫ�ڵĶ�����Ŀ\n";
	cin>>diagdoku.holes;
    diagdoku.remove_digits();
    diagdoku.copy_sudoku();
    // �ڶ��������
    cout << "\n���ɵ�����Ϊ:\n";
    diagdoku.print_modified_sudoku();

    // �������� CNF
    toCnf();

	printf("��������Ӧ��cnf�ļ���\n");
	system("pause");
		break;
	
	case 2:
		cnf=new crossCNF(file);//���³�ʼ��
		cnf->solve("Diagdoku.res","Diagdoku.cnf",true);
        file2=fopen("Diagdoku.res","r");
    // ��ȡ��һ��
        fgets(line1, sizeof(line1), file2);
    // ��ȡ�ڶ���ͷ�����ַ�
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
    // �ر��ļ�
    	fclose(file2);
        cout << "��׼��:\n";
        print_sudoku(sudoku);
        printf("SAT���𰸣�\n");
        for(int i=0;i<N;i++)
        {
            for(int j=0;j<N;j++)
            printf("%d ",ans[i][j]);
            printf("\n");
        }
		printf("�������");
		break;
    case 3:
        newsudoku=modified_sudoku;
        cout<<"��ǰ��������"<<endl;
        print_sudoku(newsudoku);
        cout<<"��������������������λ�ã�i��j�����Լ����������num��ֻ����1��0�˳���д��"<<endl;
        cin>>row;
        while(row!=0)
        {
            cin>>col>>num;
            newsudoku[row-1][col-1]=num;
            cout<<"��ǰ��������"<<endl;
            print_sudoku(newsudoku);
            if(num!=sudoku[row-1][col-1])
            cout<<"��д����"<<endl;
            cout<<"������д������(i,j,num),����д������0"<<endl;
            cin>>row;
        }
        break;
    case 4:
        if(newsudoku==sudoku)
        cout<<"��д��ȷ�ˣ�"<<endl;
        else
        cout<<"��д����Ŷ��������һ�ΰɣ�";
        cout<<"����д�Ľ��"<<endl;
        print_sudoku(newsudoku);
        cout<<"��׼��"<<endl;
        print_sudoku(sudoku);
        break;

	default:
		break;
	}
    choice=inputOrder(0,4,
			"||^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
			"|| ѡ���ܣ�                ||\n"
			"|| 1.��������                ||\n"
			"|| 2.����SAT������������    ||\n"
            "|| 3.�����������             ||\n"
            "|| 4.�����                 ||\n"
			"|| 0.�˳�                    ||\n"
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
			"|| ѡ���ܣ�            ||\n"
			"|| 1.SAT���  ģ��       ||\n"
			"|| 2.Diagdoku ģ��       ||\n"
			"|| 0.�˳�                ||\n"
			"||^^^^^^^^^^^^^^^^^^^^^^^||\n"
			);
	
		switch(choice)
		{
			case 0:{
				return 0;//��ʱ�˳����ǽ����������� 
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