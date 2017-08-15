/*************************************************************************
    > File Name: client.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: 2017年08月07日 星期一 09时59分13秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

int doread(int fd,char *buf, int size)
{
	int readline = 0;
	while(readline != size)
	{
		int ret = read(fd,buf + readline,size - readline);
		if(ret > 0)
		{
			readline += ret;
		}
		else if(ret < 0 && errno == EINTR)
		{	
			continue;
		}
		else 
		{
			break;
		}
	}
	return readline;
}

void *pthread_func(void *args)
{
	int fd = (int)args;
	while(1)
	{
		uint32_t header;
		int ret = doread(fd,(char *)&header,4);
		if(ret != sizeof(header))
		{
			exit(1);
		}
		header = ntohl(header);
		char *buf = malloc(header + 1);
		buf[header] = 0;
		ret = doread(fd,buf,header);
		if(ret != header)
		{
			exit(1);
		}
		printf("%s\n",buf);
	}
}

int main()
{
	int fd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	addr.sin_addr.s_addr = inet_addr("192.168.40.133");
	connect(fd,(struct sockaddr*)&addr,sizeof(addr));
	
	pthread_t tid;
	pthread_create(&tid,NULL,pthread_func,(void *)fd);

	while(1)
	{
		char buf[4096];
		fgets(buf,sizeof(buf),stdin);
		buf[strlen(buf) - 1] = 0;

		if(strlen(buf) == 0)
		{
			continue;
		}
		uint32_t header = strlen(buf);
		header = htonl(header);
		write(fd,&header,4);
		write(fd,buf,strlen(buf));
	}
	pthread_join(tid,NULL);
	return 0;
}
