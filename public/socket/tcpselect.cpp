#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <sys/select.h>
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
    if (argc != 2) { printf("usage: ./tcpselect port\n"); return -1; }
    int listenfd = initserver(atoi(argv[1]));
    if(-1 == listenfd)
    {
        printf("initserver() failed...\n");
        return -1;
    }

    fd_set readfds; // 读事件socket的集合，包括监听socket和客户端连接上来的socket
    FD_ZERO(&readfds);
    FD_SET(listenfd, &readfds); // 把listensock添加到读事件socket的集合中,即将listenfd位置1

    int maxfd = listenfd; // 记录集合中socket的最大值
    while(true)
    {
     // 事件：1)新客户端的连接请求accept；2)客户端有报文到达recv，可以读；3)客户端连接已断开；
    //       4)可以向客户端发送报文send，可以写。
    // 可读事件  可写事件
    // select() 等待事件的发生(监视哪些socket发生了事件)。
    
        fd_set tmpfds = readfds; // 创建一个读事件socket的集合的本
        struct timeval timeout;
        timeout.tv_sec=10; 
        timeout.tv_usec=0;
        int indfs = select(maxfd + 1, &tmpfds, NULL, NULL, &timeout);
        
        if(indfs < 0)
        {
            printf("select() failed...\n");
            break;
        }

        if(indfs == 0)
        {
            printf("select() timeout...\n");
            continue;
        }

        //如果indfs > 0 表示有事件发生的socket数量
        for(int eventfd = 0; eventfd <= maxfd; eventfd++)
        {
            if(FD_ISSET(eventfd, &tmpfds) == 0) 
            {
                continue; // 如果没有事件，continue
            }

            // 如果发生事件的是listensock，表示有新的客户端连上来
            if(eventfd == listenfd)
            {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientfd = accept(listenfd, (struct sockaddr*)&client, &len);
                printf ("accept client(socket=%d) ok.\n",clientfd);

                //把新连接的客户端加入可读socket集合
                FD_SET(clientfd, &readfds);  //注意这里不要用副本
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
                    FD_CLR(eventfd, &readfds); // 把已关闭客户端的socket从可读socket的集合中删除。
                    // 重新计算maxfd的值，注意，只有当eventfd==maxfd时才需要计算。
                    if (eventfd == maxfd) 
                    {
                        for(int i = maxfd; i >= 0; i--)
                        {
                            if(FD_ISSET(i, &readfds))
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
                     fd_set tmpfds;
                    FD_ZERO(&tmpfds);
                    FD_SET(eventfd,&tmpfds);
                    if (select(eventfd+1,NULL,&tmpfds,NULL,NULL)<=0)
                    perror("select() failed");
                    else
                    send(eventfd,buffer,strlen(buffer),0);
                    //这几行代码和直接send(eventfd,buffer,strlen(buffer),0)效果一样 没必要
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