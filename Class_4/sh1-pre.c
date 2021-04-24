#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void commandSplit(char **argv,char *command)
{

	int i,j,s,e,cnt;
	char *src = NULL;

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
}

void mysys(char *command)
{
	//实现该函数，该函数执行一条命令，并等待该命令执行结束
	char *argv[255] = {NULL};
	
	commandSplit(argv,command);
	
	//cd命令直接在父进程中执行，调用chdir函数
	if(!strcmp(argv[0],"cd"))
	{
		int ret = chdir(argv[1]);
		if(-1 == ret)
		{
			perror("cd");
			exit(0);
		}
	
		return ;
	}


	pid_t pid;
	pid = fork();//创建子进程
	if(pid == 0) {
		//调用子进程	
		int error;
		error = execvp(argv[0], argv);
	
		if(error < 0) 
		{
			perror("excel");
		}

	}

	wait(NULL);

	//释放字符数组指针
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

		//去掉换行符
		order[strlen(order)-1] = 0;
	
		//退出命令单独处理	
		if(!strcmp(order,"exit")) break;
		
		mysys(order);
	}
	
	return 0;
}
