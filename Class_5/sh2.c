#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>




//使用全局变量保存上一次的工作路径
char prePath[255] = "\0";

//将重定向>符号分割开
void split1(char *src)
{
	char *temp = (char *)malloc(sizeof(char)*255);
	memset(temp,0,sizeof(temp));
	int i=0,j=0;
	int flag=0;
	for(;src[i];i++,j++)
	{
		if(flag) 
		{
			temp[j]=' ';
			flag=0;
			i--;
			continue;
		}
		if(src[i]=='>') flag=1;
		temp[j]=src[i];
	}
	memset(src,0,strlen(src));
	strcpy(src,temp);
}

//查看是否有重定向输出符号
int findRedirectSign(char **argv)
{
	int i;
	for(i=0;argv[i];i++)
	{
		if(!strcmp(argv[i],">")) return i;
	}
	return -1;
}

//分解command命令的各个参数
int commandSplit(char **argv,char *command)
{

	int i,j,s,e,cnt;
	char *src = NULL;

	split1(command);
	cnt = 0;
	for(i = 0;i<255;i++) {
		argv[i] = NULL;
	}

	for(i = 0; i < strlen(command); i++)
	{
		if( command[i] == ' ')  continue;
		
		s=i++;
		while(command[i] != ' ' && command[i] != '\0') {
			i++;
		}
		
		e = i-1;
		src = (char *)malloc(sizeof(char) * 255);
		argv[cnt++]  = src;
		memset(src,0,255);
		for(j=0; s<=e; j++)
		{
			src[j] = command[s];
			s++;
		}
	}
	return cnt;
}

//将~符号替换为home路径
char *replaceHomePath(char *srcPath, char *homePath)
{
	char *res = NULL;
	res = (char*)malloc(strlen(srcPath)+strlen(homePath)+1);
	memset(res,0,sizeof(res));

	strcat(res,homePath);
	char *p = srcPath;

	strcat(res,p+1);

	free(srcPath);

	return res;
}

void showArgv(char **argv)
{
	int i;
	for(i=0;argv[i];i++)
	{
		printf("argv[%d]=\"%s\"\n",i,argv[i]);
	}
}

void mysys(char *command)
{
	//实现该函数，该函数执行一条命令，并等待该命令执行结束
	char *argv[255] = {NULL};
	int cnt = 0;//命令的个数

	cnt = commandSplit(argv,command);
	
	//对~进行处理,将其替换为home路径
	if(argv[1] != NULL && argv[1][0] == '~')
	{
		struct passwd *user;
		user= getpwuid(getuid());
		argv[1] = replaceHomePath(argv[1],user->pw_dir);
	}

	//cd命令直接在父进程中执行，调用chdir函数
	if(!strcmp(argv[0],"cd"))
	{
		int ret = -1;
		//使用一个临时变量存储prePath
		char *tempPath = (char*)malloc(strlen(prePath)+1);
		memset(tempPath,0,sizeof(tempPath));
		strcat(tempPath,prePath);
		//保存当前的工作路径
		memset(prePath,0,sizeof(prePath));
		getcwd(prePath,sizeof(prePath));

		if(!strcmp(argv[1],"-"))   ret = chdir(tempPath);	//处理cd -.
		else ret = chdir(argv[1]);
		free(tempPath);

		if(-1 == ret)
		{
			perror("cd");
			exit(0);
		}
		return ;
	}
	
	pid_t pid;
	pid = fork();//创建子进程
	if(pid == 0) {  //调用子进程
		//先判断是否存在重定向输入符号
		int ret = findRedirectSign(argv);
		if(ret != -1)
		{
			//如果存在,打开重定向输出的文件,将更新文件描述符
			int fd=open(argv[ret+1],O_CREAT|O_RDWR,0666);
			dup2(fd,1);
			//将后面多余的参数去掉
			int i=ret;
			for(;argv[i];i++)
			{
				argv[i]=NULL;
			}	
		}
	
		int error;
		error = execvp(argv[0], argv);
		if(error < 0) 
		{
			perror("execvp");
		}
	}
	wait(NULL);
	//释放字符数组指针
	int i=0;
	for(i=0;i<cnt;i++)
	{
		free(argv[i]);
	}
}

int main()
{
	char order[255];
	while(1)
	{
		printf("> ");
		memset(order,0,255);
		fgets(order,255,stdin);

		//回车单独处理
		if(!strcmp(order,"\n")) continue;
		
		//去掉换行符
		order[strlen(order)-1] = 0;
	
		//退出命令单独处理	
		if(!strcmp(order,"exit")) break;

		mysys(order);
	}
	return 0;
}
