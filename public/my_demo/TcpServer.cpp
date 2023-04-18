//此程序用于演示采用TcpClient类实现socket通讯的服务端

#include "../_public.h"
#include <cstdio>
#include <cstring>

#define BUFSIZE 10240

int main(int argc, char* argv[])
{
    if(argc !=2) 
    {
        cout<<"usage: ./TcpServer <port>"<<endl;
        return -1;
    }

    CTcpServer tcpServer;

    //服务端初始化
    if(!tcpServer.InitServer(atoi(argv[1])))
    {
        printf("tcpServer.InitServer(%s) failed.\n",argv[1]); 
        return -1;
    }

    //阻塞直到接收客户端的连接
    if(!tcpServer.Accept())
    {
        printf("tcpServer.Accept()\n");
        return -1;
    }

    printf("客户端：%s 已经连接\n", tcpServer.GetIP());

    char buffer[BUFSIZE];

    //与客户端通讯，接收客户端发过来的报文再回复
    while (true) 
    {
        memset(buffer, 0, BUFSIZE);
        if(!tcpServer.Read(buffer))
        {
            break;
        }
        printf("接收到: %s\n", buffer);

        strcpy(buffer, "OK");
        if(!tcpServer.Write(buffer)) break;
        printf("服务端发送: %s\n", buffer);
    }
}