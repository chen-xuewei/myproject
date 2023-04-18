#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <poll.h>
#include <sys/epoll.h>

#define MAXLOG 5
#define MaxSize 1024

// 初始化服务端的监听端口。
int initserver(int port);

int main(int argc, char* argv[])
{
    if (argc != 2) { printf("usage: ./tcppoll port\n"); return -1; }
    int listenfd = initserver(atoi(argv[1]));
    if(-1 == listenfd)
    {
        printf("initserver() failed...\n");
        return -1;
    }
    printf ("listenfd(socket=%d) ok.\n", listenfd);

    struct pollfd fdsets[MaxSize]; // I/O事件的集合数组
    for(int i = 0; i < MaxSize; i++)
    {
        fdsets[i].fd = -1; // 全部初始化为-1；
    }

    fdsets[listenfd].fd = listenfd;
    fdsets[listenfd].events = POLLIN;
    
    int maxfd = listenfd; // 记录集合中socket的最大值
    while(true)
    {
  
    
        //第二个参数是
        int indfs = poll(fdsets, maxfd +1, 10000);
        if(indfs < 0)
        {
            printf("poll() failed...\n");
            break;
        }

        if(indfs == 0)
        {
            printf("poll() timeout...\n");
            continue;
        }

        //如果indfs > 0 表示有事件发生的socket数量
        for(int eventfd = 0; eventfd <= maxfd; eventfd++)
        {
            if(fdsets[eventfd].fd < 0 || (fdsets[eventfd].revents & POLLIN) == 0) 
            {
                continue; // 如果fd为负或没有事件就忽略，continue
            }

            fdsets[eventfd].revents = 0;

            // 如果发生事件的是listensock，表示有新的客户端连上来
            if(eventfd == listenfd)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientfd = accept(listenfd, (struct sockaddr*)&client, &len);
                printf ("accept client(socket=%d) ok.\n",clientfd);

                //把新连接的客户端加入可读socket集合
                fdsets[clientfd].fd = clientfd;
                fdsets[clientfd].events = POLLIN;
                fdsets[clientfd].revents = 0;

                if(maxfd < clientfd)
                {
                    maxfd = clientfd; // 更新maxfd的值
                }
            }
            else
            {
                // 如果是客户端连接的socke有事件，表示有报文发过来或者连接已断开
                char buffer[512];
                memset(buffer, 0, sizeof(buffer));
                if(recv(eventfd, buffer, sizeof(buffer), 0) <= 0)
                {
                    // 如果客户端的连接已断开。
                    printf("client(eventfd=%d) disconnected.\n",eventfd);
                    close(eventfd);  // 关闭客户端的socket
                    fdsets[eventfd].fd = -1; // 把已关闭客户端的socket从可读socket的集合中删除。
                    // 重新计算maxfd的值，注意，只有当eventfd==maxfd时才需要计算。
                    if (eventfd == maxfd) 
                    {
                        for(int i = maxfd; i >= 0; i--)
                        {
                            if(fdsets[i].fd != -1)
                            {
                                maxfd = i;
                                break; 
                            }
                        }
                    }
                }
                else
                {
                   // 如果客户端有报文发过来
                    printf("recv(eventfd=%d):%s\n",eventfd,buffer);
                    // 把接收到的报文内容原封不动的发回去。
                    send(eventfd,buffer,strlen(buffer),0);
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