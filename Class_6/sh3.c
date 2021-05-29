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

#define MAX_ARGC 16
#define MAX_COMMANDS 5

struct command {
	int argc;
	char *argv[MAX_ARGC];
	char *input; // 用于重定向输入
	char *outpu; // 用于重定向输出
};

//全局变量
char prePath[255]="\0";//使用保存上一次的工作路径
int command_count=0;//每一次的命令个数
struct command commands[MAX_COMMANDS];//命令结构体数组

//函数声明
int commandSplit(char **argv,char *command);
char *replaceHomePath(char *srcPath, char *homePath);//将~符号替换为home路径

//输出comman的信息,用于检测parse_command()函数实现是否正确
void command_dump(struct command command)
{
	//printf("command_dump begin\n");
	int i;
	printf("argc=%d  ",command.argc);
	printf("argv=");
	for(i=0;command.argv[i];i++)
	{
		printf("'%s' ",command.argv[i]);
	}
	printf("\n");
}
//分解管道隔开的每一个命令参数
void parse_command(char *line,struct command* command)
{
	command->argc=commandSplit(command->argv,line);
	
	//对~进行处理,将其替换为home路径
	int i=0;
	for(;i<command->argc;i++)
	{
		if(command->argv[i][0] == '~')
		{
			struct passwd *user;
			user=getpwuid(getuid());
			command->argv[i] = replaceHomePath(command->argv[i],user->pw_dir);
		}
	}
}
//分解每一次输入的命令
void parse_commands(char *line)
{
	//printf("prase_command begin\n");
	char *str;
	command_count=0;

	str=strtok(line,"|");
	while(str != NULL)
	{
		//printf("str='%s'\n",str);
		parse_command(str, &commands[command_count]);
		//command_dump(commands[command_count]);
		command_count++;	
		str=strtok(NULL, "|");
	}
}
//将重定向符号'>'和'<'分隔开
void splitRedirectSign(char *src)
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
		if(src[i]=='>' || src[i]=='<') flag=1;
		temp[j]=src[i];
	}
	memset(src,0,strlen(src));
	strcpy(src,temp);
}
//查看是否有重定向输入或输出符号
int findRedirectSign(char **argv, char *sign)
{
	int i;
	for(i=0;argv[i];i++)
	{
		if(!strcmp(argv[i],sign)) return i;
	}
	return -1;
}
//分解command命令的各个参数,并返回参数的个数
int commandSplit(char **argv,char *command)
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

void exec_simple(struct command command)
{
	int ret;
	//判断是否存在重定向输出符号
	ret = findRedirectSign(command.argv,">");
	if(ret != -1)
	{
		//如果存在,打开重定向输出的文件,并更新文件描述符
		int fd=open(command.argv[ret+1],O_CREAT|O_RDWR,0666);
		dup2(fd,1);
		close(fd);
		//将后面多余的参数去掉
		int i=ret;
		for(;command.argv[i];i++)
		{
			command.argv[i]=NULL;
		}	
	}
	//判断是否存在重定向输入符号
	ret = findRedirectSign(command.argv,"<");
	if(ret != -1)
	{
		//如果存在,打开重定向输入的文件,并更新文件描述符
		int fd=open(command.argv[ret+1],O_CREAT|O_RDWR,0666);
		dup2(fd,0);
		close(fd);
		//将后面多余的参数去掉
		int i=ret;
		for(;command.argv[i];i++)
		{
			command.argv[i]=NULL;
		}	
	}
	
	int error;
	error = execvp(command.argv[0], command.argv);
	if(error < 0) 
	{
		perror("execvp");
	}
}

void exec_pipe()
{
	int fd_array[2];
	int pid;
	if(command_count > 1)
	{
		pipe(fd_array);
		pid=fork();
		if(pid==0)
		{
			close(fd_array[1]);
			dup2(fd_array[0],0);
			close(fd_array[0]);
			exec_simple(commands[1]);
		}
		close(fd_array[0]);
		dup2(fd_array[1],1);
		close(fd_array[1]);
		exec_simple(commands[0]);
	}
	else exec_simple(commands[0]);
}

void mysys(char *command)
{
	splitRedirectSign(command);
	parse_commands(command);	
	//cd命令直接在父进程中执行，调用chdir函数
	if(!strcmp(commands[0].argv[0],"cd"))
	{
		int ret = -1;
		//使用一个临时变量存储prePath
		char *tempPath = (char*)malloc(strlen(prePath)+1);
		memset(tempPath,0,sizeof(tempPath));
		strcat(tempPath,prePath);
		//保存当前的工作路径
		memset(prePath,0,sizeof(prePath));
		getcwd(prePath,sizeof(prePath));

		if(!strcmp(commands[0].argv[1],"-"))   ret = chdir(tempPath);	//处理cd -.
		else ret = chdir(commands[0].argv[1]);
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
	if(pid == 0) 
	{//调用子进程
		exec_pipe();
	}
	wait(NULL);
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
