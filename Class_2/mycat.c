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


void catfile(char *src) {
	int fd;
	
	char str[255];
	memset(str,'\0',255);
	
	fd = open(src, O_RDONLY);
	if(fd < 0 ) panic("open");
	int num;
	num = read(fd,str,sizeof(str));
	while(num > 0) {
		printf("%s", str);
		memset(str, '\0', 255);
		num = read(fd, str, sizeof(str));
	}
	num = close(fd);
	
	if(num < 0) panic("close");
	return ;
}

int main(int argc,char* argv[]) {
	if(argc < 2) {
		printf("参数不够!!\n");
		return 0;
	}
	int i;
	for(i = 1; i < argc; i++)  {
		catfile(argv[i]);
	}
	
	return 0;
}
