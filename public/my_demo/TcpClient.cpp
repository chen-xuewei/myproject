//此程序用于演示采用TcpClient类实现socket通讯的客户端

#include "../_public.h"


#define BUFSIZE 10240

int main(int argc,char *argv[])
{
    if (argc!=3)
    {
        printf("Usage:./TcpClient <ip> <port>\n"); 
        return -1;
    }

    CTcpClient tcpClient;
    //向服务端发起连接请求
    if(!tcpClient.ConnectToServer(argv[1], atoi(argv[2])))
    {
        printf("tcpClient.ConnectToServer(%s,%s) failed.\n",argv[1],argv[2]); 
        return -1;
    }

    char buffer[BUFSIZE];
    //与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    for(int i; i < 10000; i++)
    {
        SPRINTF(buffer, sizeof(buffer), "这是第%d个超级女生, 编号%03d。", i+1, i+1);
        if(!tcpClient.Write(buffer))
        {
            break;
        }
        printf("发送：%s\n",buffer);

        //接收服务端的回应报文
        memset(buffer, 0, BUFSIZE);
        if(!tcpClient.Read(buffer)) break;

        printf("接收：%s\n",buffer);

        //间隔1秒再发送报文
        sleep(1);
    }
}

