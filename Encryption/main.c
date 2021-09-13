#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<conio.h>
#include<time.h>

void menu(void);
void password(void);
void addcode(void);
void delcode();
void code(void);
void Encryption(void);
void Decrypt(void);

int main()
{
	menu();      //显示菜单
}

void password(void)              //程序密码验证函数
{
	char pass1[20];
	char pass2[20];
	int i=0;
	int input_count=1;
	FILE *pw;
	pw=fopen("pw.obj","ab+");     //打开密码文件   无则新建
	if(pw==NULL)
	{
		printf("文件丢失\n");
	}
	while(fread(&pass1[i],sizeof(char),1,pw))  //读取密码
		i++;
	pass1[i]='\0';
	fclose(pw);
	if(strlen(pass1))
	{
		while(input_count<4)
		{
			printf("请第%d/4次请输入密码启动程序(用回车键结束)!\n",input_count);
			
			for(i=0;i<20;)
			{
				pass2[i]=(char)getch();
				if(pass2[i]==8)
				{
					printf("\b \b");
					i--;
					pass2[i]='\0';
					continue;
				}
				if(pass2[i]==13)
				{
					pass2[i]='\0';
					break;
				}
				else
					printf("*");
				i++;
			}
			if(strcmp(pass1,pass2)==0)
				break;
			else
				printf("\n密码输入错误!\n");
			system("pause");
			system("cls");
			input_count++;
		}
		if(input_count==4)
		{
			printf("\n\n\t禁止非法登陆!!!\n\n");
			system("pause");
			exit(0);
		}
	}
	else;
	system("attrib pw.obj +h");        //增加密码文件隐藏属性
}

void addcode(void)    //添加程序密码函数
{
	char word1[20],word2[20];
	int i;
	FILE *pw;
	system("attrib pw.obj -h");  //去除密码文件隐藏属性
	printf("请输入新密码(以回车结束):");
	for(i=0;i<20;)
	{
		word1[i]=(char)getch();
		if(word1[i]==8)
		{
			printf("\b \b");
			i--;
			word1[i]='\0';
			continue;
		}
		if(word1[i]==13)
		{
			word1[i]='\0';
			break;
		}
		else
			printf("*");
		i++;
	}
	printf("\n请再次输入新密码(以回车结束):");
	for(i=0;i<20;)
	{
		word2[i]=(char)getch();
		if(word2[i]==8)
		{
			printf("\b \b");
			i--;
			word2[i]='\0';
			continue;
		}
		if(word2[i]==13)
		{
			word2[i]='\0';
			break;
		}
		else
			printf("*");
		i++;
	}
	if(strcmp(word1,word2)==0)
	{
		pw=fopen("pw.obj","wb");
		if(pw==NULL)
		{
			printf("文件丢失!\n");
		}
		fwrite(word1,1,strlen(word1),pw);
		fclose(pw);
		printf("\n密码设置成功!\n");
		system("attrib pw.obj +h");     //增加密码文件隐藏属性
		
	}
	else
	{
		printf("两次密码输入不同,密码设置失败!");
		system("attrib pw.obj +h");
	}
	system("pause");
	
}

void delcode()  //程序密码删除
{
	system("attrib pw.obj -h");
	system("del pw.obj");
	printf("密码删除成功!");
	fclose(fopen("pw.obj","wb"));  //新建密码文件
	system("attrib pw.obj +h");
	system("pause");
}



void code(void)   //程序密码设置函数
{
	unsigned short a,q;
	system("cls");
	password();
	do
	{
		system("cls");
		printf("\t\t*********请选择操作项目**************\n");
		printf("\n\t\t*****  \t1---设置密码。\t        *****");
		printf("\n\t\t*****  \t2---修改密码。\t*****");
		printf("\n\t\t*****  \t3---删除密码。\t*****");
		printf("\n\t\t*****  \t0---返回上一级。\t        *****");
		printf("\n\t\t*************************************\n");
		printf("\n\t\t请选择0--3\n\t\t");
		q=scanf("%d",&a);
		flushall();
		if(q)
		{
			switch(a)
			{
			case 1: addcode();break;   //密码设置
			case 2: addcode(); break;  //密码修改
			case 3: delcode(); break;  //密码删除
			case 0: return;
			default: 
				printf("\t\t选择错误!\n\n");
				system("pause");	//将函数执行暂停于此！按任意键继续
			}
		}
		else
		{
			printf("\t\t输入有误!\n\n");
			system("pause");
		}
	}while(1);
}

void menu(void)                            //菜单操作函数
{
	unsigned short select;
	unsigned short q;
	password();                            //验证程序密码
	do
	{
		system("cls");
		printf("\n\n\n\t\t    欢迎使用万能文件加密解密程序\n\n");
		printf("\t\t*********请选择操作项目**************");
		printf("\n\t\t*****  \t   1---加密文件\t        *****");
		printf("\n\t\t*****  \t   2---解密文件\t        *****");
		printf("\n\t\t*****  \t   3---程序密码\t        *****");
		printf("\n\t\t*****  \t   0---退出系统\t        *****");
		printf("\n\t\t*************************************\n");
		printf("\n\t\t请选择0--5\n\t\t");
		q=scanf("%d",&select);
		flushall();
		if(q)
		{
			switch(select)
			{
			case 1:
				Encryption();break;            //加密函数
			case 2:
				Decrypt(); break;              //解密函数
			case 3:
				code(); break;                 //程序密码设置函数
			case 0:
				printf("\n\t\t欢迎下次使用，再见!\n\n\n");
				system("pause");
				exit(0);
			default: printf("\t\t选择错误!\n\n");
				system("pause");
			}
		}
		else
		{
			printf("\t\t输入有误!\n\n");
			system("pause");
		}
	}while(1);
}

void Encryption(void)  //加密函数
{
	FILE *fp,*temp;
	int i=0,a=0,j=0,k=0;
	short swap1,swap2;
	unsigned char ch,strfilename[30],strtempbuff[256],pass[20],word2[20],t[22];
	unsigned char c1,c2;
	clock_t start,finish;
	double duration;
	while(1)  //输入要加密文件的名字
	{
		system("cls");
		printf("请输入要加密的文件名(加后缀名):\n");
		flushall();
		gets(strfilename);
		if(0==strlen(strfilename))
		{
			printf("输入为空,请重新输入!\n");
			system("pause");
		}
		else break;
	}
	if((fp=fopen(strfilename,"rb+"))==NULL)  
	{
		printf("文件%s不存在!\n",strfilename);
		system("pause");
		return;
	}
	if((temp=fopen("tempfile.txt","wb+"))==NULL)
	{
		printf("临时文件创建失败!\n");
		system("pause");
		return;
	}
	while(1)             //输入加密密码
	{
		system("cls");
		printf("请输入加密密码:\n");
		for(i=0;i<20;)
		{
			pass[i]=(char)getch();
			if(pass[i]==8)
			{
				printf("\b \b");
				i--;
				pass[i]='\0';
				continue;
			}
			if(pass[i]==13)
			{
				pass[i]='\0';
				break;
			}
			else
				printf("*");
			i++;
		}
		if(0==strlen(pass))
		{
			printf("密码不能为空!\n");
			system("pause");
			continue;
		}
		printf("\n请再次输入加密密码:\n");
		for(i=0;i<20;)
		{
			word2[i]=(char)getch();
			if(word2[i]==8)
			{
				printf("\b \b");
				i--;
				word2[i]='\0';
				continue;
			}
			if(word2[i]==13)
			{
				word2[i]='\0';
				break;
			}
			else
				printf("*");
			i++;
		}
		if(strcmp(pass,word2)!=0)
		{
			printf("\n两次密码输入不同,请重新输入!\n");
			system("pause");
		}
		else break;
	}
	///////////////////////////////////////////////////////////////////////////////
	k=strlen(pass);
	printf("\n请稍等，加密中...\n");
	start=clock();  //加密计时开始函数
	t[0]=(char)k;  //记录密码长度
	for(i=1;i<=k;i++)
	{
		t[i]=pass[i-1];
	}
	t[i]='\0';
	i=0;
	while(t[i]!='\0')
	{
		if(j==k)
			j=0;
		swap1=t[i];
		swap1&=255;
		swap2=pass[j];
		swap1=swap1+swap2;
		swap1=~swap1;
		fwrite(&swap1,sizeof(short),1,temp);
		j++;
		i++;
	}
	while(fread(&ch,sizeof(char),1,fp)!=0)
	{
		if(j==k)
			j=0;                                //加密算法！！！
		c1=ch;
		c2=pass[j];
		c1=c2^c1;
		fwrite(&c1,sizeof(char),1,temp);
		j++;
	}
	//////////////////////////////////////////////////////////////////////////////
	fclose(temp);
	fclose(fp);
	finish=clock();   //加密结束函数
	sprintf(strtempbuff,"del %s",strfilename);
	system(strtempbuff);
	sprintf(strtempbuff,"ren tempfile.txt %s",strfilename);
	system(strtempbuff);
	printf("加密成功!\n");
	printf("本次加密用时:\t");
	duration=(double)(finish-start)/CLOCKS_PER_SEC;  //计算加密时间
	if(0==duration)
		duration=1;
	printf("%f秒\n",duration);
	system("pause");
	return;
}


void Decrypt(void)    //解密函数
{
	FILE *fp,*temp;
	int j=0,k=0,i=0,m=0;
	short swap,f=1;
	unsigned char strfilename[30],strtempbuff[256],pass[20],word[21];
	unsigned char c1,c2;
	clock_t start,finish;
	double duration;
	while(1)
	{
		system("cls");
		printf("请输入要解密的文件名(加后缀名):\n");
		gets(strfilename);
		if(0==strlen(strfilename))
		{
			printf("输入为空,请重新输入!\n");
			system("pause");
		}
		else break;
	}
	if((fp=fopen(strfilename,"rb+"))==NULL)
	{
		printf("文件%s不存在!\n",strfilename);
		system("pause");
		return;
	}
	if((temp=fopen("tempfile.txt","wb+"))==NULL)
	{
		printf("临时文件创建失败!\n");
		system("pause");
		return;
	}
	do 
	{
		system("cls");
		printf("请输入解密密码:\n");
		for(i=0;i<20;)
		{
			pass[i]=(char)getch();
			if(pass[i]==8)
			{
				printf("\b \b");
				i--;
				pass[i]='\0';
				continue;
			}
			if(pass[i]==13)
			{
				pass[i]='\0';
				break;
			}
			else
				printf("*");
			i++;
		}
		k=strlen(pass);
		printf("\n解密中...\n");
		i=0;
		j=0;
		rewind(fp);
		while(i<=k&&fread(&swap,sizeof(short),1,fp)!=0)  //读出保存密码长度、和密码
		{
			if(j==k)
				j=0;
			swap=~swap;
			word[i]=swap-pass[j];
			j++;
			i++;
		}
		if(k!=word[0])
		{
			printf("密码错误!\n");
			system("pause");
			continue;
		}
		for(i=0,m=1;i<k;i++,m++)
		{
			if(pass[i]!=word[m])
			{
				printf("密码错误!\n");
				f=0;
				system("pause");
				break;
			}
			else
				f=1;
		}
		if(f)
			break;
	}while(1);
	start=clock();    //解密计时开始
	///////////////////////////////////////////////////////////////////////////////
	while(fread(&c1,sizeof(char),1,fp)!=0)
	{
		if(j==k)
			j=0;                                //解密算法！！！
		c2=pass[j];
		c1=c1^c2;
		fwrite(&c1,sizeof(char),1,temp);
		j++;
	}
	//////////////////////////////////////////////////////////////////////////////
	fclose(temp);
	fclose(fp);
	finish=clock();     //解密计时结束
	sprintf(strtempbuff,"del %s",strfilename);
	system(strtempbuff);
	sprintf(strtempbuff,"ren tempfile.txt %s",strfilename);
	system(strtempbuff);
	printf("解密完成!\n");
	printf("本次解密用时:\t");
	duration=(double)(finish-start)/CLOCKS_PER_SEC;
	if(0==duration)
		duration=1;
	printf("%f秒\n",duration);
	system("pause");
	return;
}