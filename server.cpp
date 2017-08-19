/*************************************************************************
    > File Name: select.cpp
    > Author: ma6174
    > Mail: ma6174@163.com
    > Created Time: 2017年08月17日 星期四 20时02分30秒
 ************************************************************************/

#include<stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;

static int epollfd;
static list<int> evfds;
static pthread_mutex_t mutex;
static sem_t sem;
static int serverfd;

void set_nonblock(int fd)
{
    int flags = fcntl(fd,F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd,F_SETFL,&flags);
}

void epoll_add(int epollfd,int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLONESHOT;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
}

void epoll_del(int epollfd,int fd)
{
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,NULL);
}

void epoll_mod(int epollfd,int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLONESHOT;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

void *thread_func(void *args)
{
    int i = 0;
    while(1)
    {
        int ret = sem_wait(&sem);
        if(ret < 0 && errno == EINTR)
        {
            continue;
        }
        else if(ret < 0)
        {
            exit(1);
        }
        pthread_mutex_lock(&mutex);
        int fd;
        if(evfds.size() > 0)
        {
            fd = *evfds.begin();
            evfds.pop_front();
        }
        else
        {
            pthread_mutex_unlock(&mutex);
            continue;
        }
        pthread_mutex_unlock(&mutex);
        if(serverfd != fd)
        {
            char buf[1024];
            ret = read(fd,buf,sizeof(buf));
            if(ret < 0 && errno == EINTR)
            {
                continue;
            }
            else if(ret < 0)
            {
                exit(1);
            }
            printf("%ld:%d:%s\n",pthread_self(),i++,buf);
        }
        else if(serverfd == fd)
        {
            while(1)
            {
                int newfd = accept(fd,NULL,NULL);
                if(newfd < 0 && errno == EAGAIN)
                {
                    break;
                }
                set_nonblock(newfd);
                epoll_add(epollfd,newfd);
            }
        }
        epoll_mod(epollfd,serverfd);
    }
}

int main()
{
    pthread_mutex_init(&mutex,NULL);
    sem_init(&sem,0,0);

    pthread_t tid;
    pthread_create(&tid,NULL,thread_func,NULL);
    pthread_create(&tid,NULL,thread_func,NULL);
    pthread_create(&tid,NULL,thread_func,NULL);
    pthread_create(&tid,NULL,thread_func,NULL);

    int fd = socket(AF_INET,SOCK_STREAM,0);
    serverfd = fd;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret < 0)
    {
        perror("bind fail");
        exit(1);
    }
    listen(fd,250);

    set_nonblock(fd);

    epollfd = epoll_create(1024);
    epoll_add(epollfd,fd);
    while(1)
    {
        struct epoll_event ev[8];
        int ret = epoll_wait(epollfd,ev,8,5000);
        if(ret > 0)
        {
            int i;
            for(i = 0; i < ret; i++)
            {
                int fd = ev[i].data.fd;
                //epoll_del(epollfd,fd);
                pthread_mutex_lock(&mutex);
                evfds.push_back(fd);
                pthread_mutex_unlock(&mutex);
                sem_post(&sem);
            }
        }
    }
    return 0;
}

