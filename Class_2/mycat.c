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

int fd;



int main(int argc,char* argv[]) {
	char str[255];
	memset(str,'\0',255);
	fd = open(argv[1],O_RDONLY);
	if(fd < 0 ) panic("open");
	int num;
	num = read(fd,str,sizeof(str));
	while(num>0) {
		printf("%s",str);
		memset(str,'\0',255);
		num = read(fd,str,sizeof(str));
	}
	num = close(fd);
	if(num < 0) panic("close");
	return 0;
}
