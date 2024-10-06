//这个文件是对solver.h中CNF求解器部分的实现 

#include<cstdlib>
#include<cstdio>
#include<ctime>
#include<new>
#include<cassert>
#include "solver.h"
using namespace std;

int random=0;//如果random==0，会使用“倾向”求解策略，否则会使用“随机”求解策略
//倾向：在分类讨论布尔变元L为真还是为假时，如果一开始的cnf中+L的出现次数比-L多则先讨论真，否则先讨论假
//随机：在分类讨论布尔变元L为真还是为假时，随机先讨论真还是先讨论假 
int my_choice;
bool crossCNF::calculate(const char * const filename)
//验证cnf和 res文件是否匹配 
{
	
	int *ans=new int[boolNum+1];//res文件的内容会被写入ans再与cnf进行比较 
	int i,n;
	
	FILE *fp=fopen(filename,"r");
	if(fp==NULL)
	{
		printf("文件打开失败，不能检验\n");
		delete[] ans;
		return 0;
	}
	fscanf(fp,"%*s%d%*s",&n);
	if(n==0)
	{
		printf("无解，不可验证\n");
		return 0;
	}
	
	printf("res=[");
	for(i=0;i<boolNum;i++)
	{
		fscanf(fp,"%d",&n);
		if(n<0)ans[-n]=0;
		else ans[n]=1;
		printf("%d ",n);
	}
	bool ok;
	crossNode *p;
	printf("]\n检验开始\n");
	for(i=1;i<=clauseNum;i++)//在每个子句中找到第一个为True的文字，找到了，这个子句就算True 
	{
		ok=false;
		p=clauses[i].right;
		while(p)
		{
			n=p->Bool;
			printf("%d ",n);
			if(n<0 && ans[-n]==0  || n>0 && ans[n]==1)
			{
				printf("True");
				ok=true;
				break;
			}
			
			
			p=p->right;
		}
		printf("\n");
		if(!ok)
		{
			printf("clause:False");//如果发现哪个子句为False，则整个cnf也就为False 
			fclose(fp);
			delete[] ans;
			return false;
		}
	}
	
	
	delete[] ans;
	fclose(fp);
	return true;
}

bool crossCNF::solve(const char* const filename,bool display)
//名义解，主要工作是为innerSolve函数创造环境，把结果写成文件什么的 
{
	
	FILE *fp=NULL;
	if(filename)fp=fopen(filename,"w");
	if(random)printf("当前求解策略：随机\n");       
	else printf("当前求解策略：倾向\n");
	
	
	int start_time=clock();//开始计时 
	bool ans=innerSolve();
	int delta_time=clock()-start_time;//结束计时 
	
	if(ans)
	{
		//有解 
		if(display)printf("有解 耗时%dms\n",delta_time);
		hypo[0]=TRUE;
		if(fp)
		{
			fprintf(fp,"s 1\nv ");
		
			for(int i=1;i<=boolNum;i++)
			{
				if(hypo[i]==TRUE)fprintf(fp,"%d ",i);
				else if(hypo[i]==FALSE)fprintf(fp,"-%d ",i);
				else fprintf(fp,"-%d ",i);
			}
			
			fprintf(fp,"\nt %d",delta_time);
		}
			
		
	}
	else
	{
		//无解
		if(display)printf("无解 耗时%d\n",delta_time);
		hypo[0]=FALSE;
		if(fp)fprintf(fp,"s 0\n");
		
	}
	fclose(fp);
	
	
	
	return ans;

}

bool crossCNF::solve(const char* const filename,const char * const filename0,bool display)//filename为输出指定文件
//名义解，主要工作是调用innerSolve函数求解，并输出结果到指定文件
{
	
	FILE *fp=NULL;
	if(filename)fp=fopen(filename,"w");
	printf("请选择你的求解策略\n"
						"||^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^||\n"
						"|| （1）.第一变元选取策略                 \n"
						"|| （2）.随机策略				\n"
						"|| （3）.最短子句出现频率最大优先策略(MOM)   \n"
						"||^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^||\n");
	scanf("%d",&my_choice);
	bool ans;
	int start_time=clock();//开始计时 
	printf("正在求解，请耐心等待……\n");
	switch (my_choice)
	{
	case 1:
	case 2:
		random=0;
		ans=innerSolve();
		break;
	case 3:
    	random=1;
		ans=innerSolve();
		break;
	default:
		break;
	}
	int delta_time=clock()-start_time;//结束计时 
	
	if(ans)
	{
		//有解 
		if(display)printf("有解 耗时%dms\n",delta_time);
		hypo[0]=TRUE;
		if(fp)
		{
			fprintf(fp,"s 1\nv ");
		
			for(int i=1;i<=boolNum;i++)
			{
				if(hypo[i]==TRUE)fprintf(fp,"%d ",i);
				else if(hypo[i]==FALSE)fprintf(fp,"-%d ",i);
				else fprintf(fp,"-%d ",i);
			}
			
			fprintf(fp,"\nt %d",delta_time);
		}
			
		
	}
	else
	{
		//无解
		if(display)printf("无解 耗时%dms\n",delta_time);
		hypo[0]=FALSE;
		if(fp)fprintf(fp,"s 0\n");
		
	}
	system("pause");
	fclose(fp);
	
	
	
	return ans;

}

int crossCNF::believe(int L)
//相信L==true，这个函数会和innerSolve相互调用 
{
	
	//在hypothesis中注册结果 
	if(L>0)hypo[L]=TRUE;
	else hypo[-L]=FALSE;//将负数转化成对应的正数bool变元编号，让它为假，即-L为真（临时result）
	//设置“被删除”栈 
	crossNode *DeleteNodesHead=NULL;//被删除的bool变元栈
	//设置临时指针变量 
	crossNode *p,*q; 
	int i,target;
	int startNum=remainClauseNum;
	
	//删去所有含L的子句 
	for(q=bools[changeBool(L)].down;q;q=q->down)//正数不动，负数化为对应的大正数，顺着向下的指针删
	{
		
		if(q->del)continue;
		target=q->Clause;
		sum[target]++;
		remainClauseNum--;
		for(p=&clauses[target];p;p=p->right)
		{
			if(p->del)continue;
			p->del=true;      // 已经被删除了 
			
			//把p加入DeleteNodes栈 
			p->next=DeleteNodesHead;  
			DeleteNodesHead=p;
			sum[target]--;
		}
	}
	
	//如果导致剩余子句数量为0 ,就认为这是真解 
	if(remainClauseNum==0)return 1;
	//需要删去所有子句中的-L 
	for(p=bools[changeBool(-L)].down;p;p=p->down)
	{
		if(p->del)continue;
		p->del=true;
		p->next=DeleteNodesHead;
		DeleteNodesHead=p;
		sum[p->Clause]--;
		// 删除多了怎么办 
		//可以在这里检查空子句
		if(sum[p->Clause]==0)
		{
			restore(L,startNum,DeleteNodesHead);
			return 0;
		}
		if(sum[p->Clause]==1)single.push(p->Clause);//如果删出了单子句，就把它加到“单子句”栈中去 
	}
	if(innerSolve())return 1;
	restore(L,startNum,DeleteNodesHead);
	return 0;
}
	
	
void crossCNF::restore(int L,int startNum,crossNode *Head)
//把believe中做出的修改复原 
{
	
	hypo[abs(L)]=UNSURE;
	while(Head)//复原一些结点 
	{
		Head->del=false;
		sum[Head->Clause]++; 
		Head=Head->next;
	}
	remainClauseNum=startNum;
}

int crossCNF::innerSolve()
//实际解，会和believe相互调用 
{
	int i,L;
	crossNode *p;
	while(!single.empty())//试图从“单子句”栈中获得单子句 
	{
		L=single.pop();
		if(!clauses[L].del && sum[L]==1)//找到单子句（可能不在子句链表开头，需要查找）
		{
			p=clauses[L].right;
			while(p->del)p=p->right;
			return believe(p->Bool);//返回这个单子句中的bool变元序号
		}
	}
	long long cnt[boolNum+10]; 
	long long max=0,index=0;
	long long min=boolNum+1;
	switch (my_choice)
	{
	case 1:	//选取“第一行第一个” 文字进行分类讨论 ， 这个策略是最普通的
	//第一文字选取策略
	for(i=1;i<=clauseNum;i++)
	{
		if(sum[i]==0) continue;   // 还剩余的文字为0 
		p=clauses[i].right;
		while(p && p->del)p=p->right;
		break;
	}
		L=abs(p->Bool);    // 转化为正数 
		break;
	case 2:	//最短子句出现频率最大优先策略(MOM)
	for(int i=1;i<=boolNum;i++)
	cnt[i]=0;
	for(i=1;i<=clauseNum;i++)
	{	
		if(sum[i]!=0)
		{
			if(sum[i]<min)
			min=sum[i];
		}
	}
	for(i=1;i<=clauseNum;i++)
	{	
		if(sum[i]==min)
		{
			p=clauses[i].right;
			while(p)
			{
				if(!p->del)
				cnt[abs(p->Bool)]++;
				p=p->right;
			}
		}
	}
	for(int i=1;i<=boolNum;i++)
	{
		if(cnt[i]>max)
		{
			max=cnt[i];
			index=i;
		}
	}
	L=index;
	break;
	case 3:	//随机策略
	for(i=1;i<=clauseNum;i++)
	{
		if(sum[i]==0) continue;   // 还剩余的文字为0 
		p=clauses[i].right;
		while(p && p->del)p=p->right;
		break;
	}
		L=abs(p->Bool);    // 转化为正数 
		break;
	default:
		break;
	}
	//开始分类讨论，首先要决定先考虑正的还是先考虑负的 
	if(random)L*=2*(rand()%2)-1;//随机化选取策略 
	else L*=tendency[L];//倾向选取策略 
	
	if(believe(L))return 1;
	return believe(-L);
	
	
}

