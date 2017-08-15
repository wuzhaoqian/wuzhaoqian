/*************************************************************************
    > File Name: pcopy.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: 2017年08月03日 星期四 22时34分56秒
 ************************************************************************/

#include<stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#define MAX_PTH 3

pthread_mutex_t mutex;

struct pthread_block
{
	int fdin;
	int fdout;
	int start;
	int end;
};

int get_size(const char *filename)
{
	struct stat file_stat;
	stat(filename,&file_stat);
	return file_stat.st_size;
}

void *pthread_func(void *args)
{
	struct pthread_block *block = (struct pthread_block *)args;
	char buf[1024];
	int count = block->start;
	int ret;
	printf("inthread = %ld,start = %d,end = %d\n",pthread_self(),block->start,block->end);
	pthread_mutex_lock(&mutex);
	lseek(block->fdin,block->start,SEEK_SET);
	lseek(block->fdout,block->start,SEEK_SET);
	while(count < block->end)
	{
		ret = read(block->fdin,buf,sizeof(buf));
		count += ret;
		write(block->fdout,buf,ret);
	}
	pthread_mutex_unlock(&mutex);
	printf("---pthread exit %ld\n",pthread_self());
	pthread_exit(NULL);
}

int main(int argc,char *argv[])
{
	if(argc < 2)
	{
		printf("argument error");
		exit(1);
	}
	pthread_mutex_init(&mutex,NULL);
	char *filename = argv[1];
	int fd = open(filename,O_RDONLY);
	int cpfd = open("copy.mp4",O_CREAT | O_WRONLY | O_TRUNC,0777);
	int size = get_size(filename);
	int pthread_count = MAX_PTH;
	int down_size = size / pthread_count;
	struct pthread_block *blocks = (struct pthread_block *)malloc(sizeof(struct pthread_block)*pthread_count);
	printf("filesize = %d,pthread_count = %d\n",size,pthread_count);
	int i;
	for(i = 0; i < pthread_count; i++)
	{
		blocks[i].fdin = fd;
		blocks[i].fdout = cpfd;
		blocks[i].start = i*down_size;
		blocks[i].end = blocks[i].start + down_size;
	}
	blocks[i-1].end = size;
	for(i = 0; i < pthread_count; i++)
	{
		printf("---%d---%d\n",blocks[i].start,blocks[i].end);
	}
	pthread_t tid[3];
	for(i = 0; i < pthread_count; i++)
	{
		pthread_create(&tid[i],NULL,pthread_func,&blocks[i]);
	}

	for(i = 0; i < pthread_count; i++)
	{
		pthread_join(tid[i],NULL);
	}
	close(fd);
	close(cpfd);
	free(blocks);
	return 0;
}
