#include <asm-generic/errno-base.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>


#define MAXLOG 5

// 初始化服务端的监听端口。
int initserver(int port);

int main(int argc, char* argv[])
{
    if (argc != 2) { printf("usage: ./tcp_epoll port\n"); return -1; }
    int listenfd = initserver(atoi(argv[1]));
    if(-1 == listenfd)
    {
        printf("initserver() failed...\n");
        return -1;
    }

    int epollfd = epoll_create(1);

    //为监听的socket准备可读事件
    struct epoll_event ev; //声明事件的数据结构
    ev.events = EPOLLIN;  // 表明读事件
    ev.data.fd = listenfd; //指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。

    //控制某个epoll监控的文件描述符上的事件：注册、修改、删除。
    //把监听的socket的事件加入epollfd中。
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    //存放epoll返回的事件
    struct epoll_event evs[10];

    while(true)
    {

        int indfs = epoll_wait(epollfd, evs, 10, -1);
        
        if(indfs < 0)
        {
            printf("epoll() failed...\n");
            break;
        }

        if(indfs == 0)
        {
            printf("epoll() timeout...\n");
            continue;
        }

        //如果indfs > 0 表示有事件发生的socket数量
        // 遍历epoll返回的已发生事件的数组evs。
        for(int indx = 0; indx < indfs; indx++)
        {
            printf("events=%d, data.fd=%d\n",evs[indx].events, evs[indx].data.fd);

            //如果发生的事件是listensock, 表示有新的客户端连上来
            if(evs[indx].data.fd == listenfd)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientfd = accept(listenfd, (struct sockaddr*)&client, &len);
                printf ("accept client(socket=%d) ok.\n",clientfd);

                //把新连接的客户端准备可读事件，并加入epoll中
                ev.data.fd = clientfd;
                ev.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
            }
            else
            {
                // 如果是客户端连接的socke有事件，表示有报文发过来或者连接已断开
                char buffer[512];
                memset(buffer, 0, sizeof(buffer));
                if(recv(evs[indx].data.fd, buffer, sizeof(buffer), 0) <= 0)
                {
                    // 如果客户端的连接已断开。
                    printf("client(eventfd=%d) disconnected.\n", evs[indx].data.fd);
                    close(evs[indx].data.fd);  // 关闭客户端的socket
                }
                else
                {
                   // 如果客户端有报文发过来
                    printf("recv(eventfd=%d):%s\n", evs[indx].data.fd, buffer);
                    // 把接收到的报文内容原封不动的发回去。
                    send(evs[indx].data.fd,buffer,strlen(buffer),0);
                }
            }
        }
    }
}

int initserver(int port)
{
    int connd;
    if(-1 == (connd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        perror("socket() failed...");
        return -1;
    }

    int opt = 1;
    setsockopt(connd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if(-1 == bind(connd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
    {
        perror("bind() failed....");
        return -1;
    }

    if(-1 == listen(connd, MAXLOG))
    {
        perror("listen() failed....");
        return -1;
    }

    return connd;
}