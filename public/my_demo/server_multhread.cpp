/*
 * 此程序演示采用开发框架的CTcpServer类实现socket通讯多进程的服务端。
 * 1）在多进程的服务程序中，如果杀掉一个子进程，和这个子进程通讯的客户端会断开，但是，不
 *    会影响其它的子进程和客户端，也不会影响父进程。
 * 2）如果杀掉父进程，不会影响正在通讯中的子进程，但是，新的客户端无法建立连接。
 * 3）如果用killall+程序名，可以杀掉父进程和全部的子进程。
 *
 * 多进程网络服务端程序退出的三种情况：
 * 1）如果是子进程收到退出信号，该子进程断开与客户端连接的socket，然后退出。
 * 2）如果是父进程收到退出信号，父进程先关闭监听的socket，然后向全部的子进程发出退出信号。
 * 3）如果父子进程都收到退出信号，本质上与第2种情况相同。
 *
*/
#include "../_public.h"


CLogFile logfile;      // 服务程序的运行日志。
CTcpServer TcpServer;  // 创建服务端对象。

void* thread_func(void* arg);

int main(int argc, char* argv[])
{
    if (argc!=3)
    {
        printf("Example:./server 5051 /home/chen/log/server.log\n"); 
        return -1;
    }


    if(!logfile.Open(argv[2], "a+"))
    {
        cout<<"logfile.Open() failed."<<argv[2]<<endl;
        return -1;
    }

    //服务端初始化
    if(!TcpServer.InitServer(atoi(argv[1])))
    {
        logfile.Write("TcpServer.InitServer(%s) failed.\n",argv[1]);
        return -1;
    }

    while(true)
    {
        if(!TcpServer.Accept())
        {
            logfile.Write("TcpServer.Accept() failed.\n");
            return -1;
        }
        logfile.Write("客户端 %s 已连接 \n", TcpServer.GetIP());
        pthread_t tid;
        pthread_create(&tid, NULL, thread_func, (void*)(long)TcpServer.m_connfd);
        pthread_detach(tid);
    }
}


// 父进程退出函数。
void FathEXIT(int sig)
{
    // 以下代码是为了防止信号处理函数在执行的过程中被信号中断。
    signal(SIGINT,SIG_IGN); 
    signal(SIGTERM,SIG_IGN);

    logfile.Write("父进程退出, sig=%d。\n",sig);
    TcpServer.CloseListen();    // 关闭监听的socket。
    kill(0, 15);   // 通知全部的子进程退出。

    exit(0);
}

// 子进程退出函数。
void ChldEXIT(int sig)  
{
    // 以下代码是为了防止信号处理函数在执行的过程中被信号中断。
    signal(SIGINT,SIG_IGN); 
    signal(SIGTERM,SIG_IGN);

    logfile.Write("子进程退出, sig=%d。\n",sig);
    TcpServer.CloseClient();    // 关闭客户端的socket。
    exit(0);
}

void* thread_func(void* arg)
{
    int ibuflen;
    char buffer[1024];
    while(true)
    {
        memset(buffer, 0, sizeof(buffer));
        if(!TcpRead((int)(long)arg, buffer, &ibuflen, 30)) break;
        logfile.Write("接收: %s\n", buffer);

        strcpy(buffer, "OK");
        if(!TcpWrite((int)(long)arg, buffer)) break;
        logfile.Write("发送: %s\n", buffer);
    }
}
