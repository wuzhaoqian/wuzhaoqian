/*************************************************************************
    > File Name: libevent.c
    > Author: ma6174
    > Mail: ma6174@163.com
    > Created Time: 2017年08月14日 星期一 17时08分36秒
 ************************************************************************/

#include<stdio.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event2/bufferevent.h>

void readcb(struct bufferevent *bev, void *ctx)
{
    char buf[1024];
    bufferevent_read(bev,buf,sizeof(buf));
    printf("buf = %s\n",buf);
    bufferevent_write(bev,"from server",12);
}

void eventcb(struct bufferevent *bev, short what, void *ctx)
{
    printf("error %d\n",(short)what);
    bufferevent_free(bev);
}

void doAccept(struct evconnlistener *server, evutil_socket_t sock,
              struct sockaddr *addr, int socklen, void *args)
{
    struct event_base *base = (struct event_base *)args;
    struct bufferevent *bev =  bufferevent_socket_new(base,sock,
                                                      BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev,readcb,NULL,eventcb,base);
    bufferevent_enable(bev,EV_READ | EV_WRITE);
}

int main()
{
    struct event_base *base = event_base_new();
    int flags = LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = INADDR_ANY;
    evconnlistener_new_bind(
                base,
                doAccept,
                base,
                flags,
                250,
                (struct sockaddr *)&addr,
                sizeof(addr)
                );

    event_base_dispatch(base);
    return 0;
}
