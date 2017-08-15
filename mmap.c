/*************************************************************************
    > File Name: mmap.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: 2017年08月01日 星期二 10时18分12秒
 ************************************************************************/

#include<stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	int fd = open("a.txt",O_RDWR);

	void *ptr = mmap(NULL,4096,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
	pid_t pid;
	pid = fork();

	if(pid == 0)
	{
		strcpy((char *)ptr,"hello world");
	}
	else if(pid > 0)
	{
		sleep(1);
		printf("%s\n",(char *)ptr);
		wait(NULL);
	}

	return 0;
}
