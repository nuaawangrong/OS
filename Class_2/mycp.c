#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


void panic(char* message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int fdSrc;
int fdRes;

int main(int agrc,char* argv[]) {
	if(agrc < 3) panic("argv is not full");
	
	char str[255];
	memset(str,'\0',255);
	fdSrc = open(argv[1],O_RDONLY);
	if(fdSrc < 0) panic("open Src");
	
	mode_t mode = 0777;
	fdRes = open(argv[2],O_RDWR | O_APPEND | O_CREAT, mode);
	if(fdRes < 0) panic("open Res");
	
	int num;
	num = read(fdSrc,str,sizeof(str));
	while(num > 0) {
		write(fdRes,str,sizeof(str));
		memset(str,'\0',255);
		num = read(fdSrc,str,sizeof(str));
	}
	close(fdSrc);
	close(fdRes);

	return 0;
}
