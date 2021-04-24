#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void mysys(char *command)
{
	//实现该函数，该函数执行一条命令，并等待该命令执行结束
	int i,j,s,e,cnt;
	char *src = NULL;
	char *argv[255];
	
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

	if(!strcmp(argv[0],"cd"))//cd命令在父进程中执行，调用chdir函数
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
	pid = fork();
	if(pid == 0) {
		//创建子进程
		int error;
		error = execvp(argv[0], argv);
	
		if(error < 0) 
		{
			perror("excel");
		}


	}
	wait(NULL);

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
		order[strlen(order)-1] = 0;//去掉换行符
		
		if(!strcmp(order,"exit")) break;
		
		mysys(order);
	}
	
	return 0;
}
