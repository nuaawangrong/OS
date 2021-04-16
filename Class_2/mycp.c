#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <utime.h>

void panic(char* message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int fdSrc;
int fdRes;

int main(int agrc,char* argv[]) {
	if(agrc != 3) panic("argv error\n");	
	char str[255];

	//首先判断两个文件是否重名
	if(0 == strcmp(argv[1],argv[2])) {
		printf("文件已存在！！！\n");
		return 1;
	}


	memset(str,'\0',255);
	fdSrc = open(argv[1],O_RDONLY);
	if(fdSrc < 0) panic("open Src");
	
	mode_t mode = 0777;
	fdRes = open(argv[2],O_RDWR | O_APPEND | O_CREAT, mode);
	if(fdRes < 0) panic("open Res");
	
	int num;
	num = read(fdSrc,str,sizeof(str));
	while(num > 0) {
		write(fdRes,str,strlen(str));
		memset(str,'\0',255);
		num = read(fdSrc,str,sizeof(str));
	}

	close(fdSrc);
	close(fdRes);

	//获取源文件属性
	struct stat src_stat;
    	stat(argv[1], &src_stat);
 
       //修改目标文件时间
       struct utimbuf timbuf;
       timbuf.actime = src_stat.st_atime;	//访问时间
       timbuf.modtime = src_stat.st_mtime;      //修改时间
       utime(argv[2], &timbuf);
 
       //修改权限
       chmod(argv[2], src_stat.st_mode);
 
       // 修改所有者
       int ret = chown(argv[2], src_stat.st_uid, src_stat.st_gid);
       if(ret < 0)
       {
	       perror("chown");
       }
       return 0;
}
