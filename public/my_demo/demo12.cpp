/*
 * 程序名：demo12.cpp，此程序用于演示网银APP软件的服务端，增加了心跳报文。
 * 从server程序改写而来
*/
#include "../_public.h"

CLogFile logfile;      // 服务程序的运行日志。
CTcpServer TcpServer;  // 创建服务端对象。

void FathEXIT(int sig);  // 父进程退出函数。
void ChldEXIT(int sig);  // 子进程退出函数。

bool bsession=false;     // 客户端是否已登录：true-已登录;false-未登录或登录失败。

// 处理业务的主函数。
bool _main(const char *recvbuffer, char *sendbuffer);

// 心跳。
bool srv000(const char *recvbuffer,char *sendbuffer);

// 登录业务处理函数。
bool srv001(const char *recvbuffer, char *sendbuffer);

// 查询余额业务处理函数。
bool srv002(const char *recvbuffer, char *sendbuffer);


int main(int argc, char* argv[])
{
    if (argc!=4)
    {
        printf("Example:./server 5051 /home/chen/log/server.log 20\n"); 
        return -1;
    }
    //进程id
    pid_t pid;

    // 关闭全部的信号和输入输出。
    // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
    // 但请不要用 "kill -9 +进程号" 强行终止
    CloseIOAndSignal();
    signal(SIGINT, FathEXIT);
    signal(SIGTERM, FathEXIT);

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
            FathEXIT(-1);
        }

        logfile.Write("客户端 %s 已连接 \n", TcpServer.GetIP());

        pid = fork();
        if(pid > 0)
        {
            //父进程空间
            TcpServer.CloseClient();
            continue;
        }
        else if (pid == 0) 
        {
            //子进程空间
            signal(SIGINT,ChldEXIT); 
            signal(SIGTERM,ChldEXIT);
            //关闭监听套接字
            TcpServer.CloseListen();

            // 子进程与客户端进行通讯，处理业务。
            char recvbuffer[1024], sendbuffer[1024];
            while(true)
            {
                memset(recvbuffer,0,sizeof(recvbuffer));
                memset(sendbuffer,0,sizeof(sendbuffer));
                if(!TcpServer.Read(recvbuffer, atoi(argv[3]))) break;
                logfile.Write("接收：%s\n", recvbuffer);

                //处理业务的主函数    
                if(!_main(recvbuffer, sendbuffer)) break;
                
                if (!TcpServer.Write(sendbuffer)) break;
                logfile.Write("发送：%s\n", sendbuffer);
            }
             ChldEXIT(0);
        }
        else
        {
            logfile.Write("fork() failed\n");
            FathEXIT(-1);
        }
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

bool _main(const char *recvbuffer, char *sendbuffer)
{
    // 解析recvbuffer，获取服务代码（业务代码）
    int isrvcode = -1;
    GetXMLBuffer(recvbuffer, "srvcode", &isrvcode);

    if((isrvcode != 1) && (bsession == false))
    {
        strcpy(sendbuffer,"<retcode>-1</retcode><message>用户未登录。</message>"); 
        return true;
    }

    //处理每种业务
    switch (isrvcode) 
    {
        case 0:
            srv000(recvbuffer, sendbuffer); 
            break;
        case 1:
            srv001(recvbuffer, sendbuffer); 
            break;
        case 2:
            srv002(recvbuffer, sendbuffer); 
            break;
        default:
            logfile.Write("业务代码不合法", recvbuffer);
            return false;
    }

    return true;
}

bool srv000(const char *recvbuffer,char *sendbuffer)
{
    strcpy(sendbuffer, "<retcode>0</retcode><message>成功。</message>");
    return true;
}

// 登录业务处理函数。
bool srv001(const char *recvbuffer, char *sendbuffer)
{
    // <srvcode>1</srvcode><tel>1392220000</tel><password>123456</password>
    char tel[21], password[21];
    GetXMLBuffer(recvbuffer,"tel",tel,20);
    GetXMLBuffer(recvbuffer,"password",password,20);

    // 把处理结果生成sendbuffer
    if((strcmp(tel, "18581765402") == 0) && (strcmp(password, "123456") == 0))
    {
        strcpy(sendbuffer,"<retcode>0</retcode><message>成功。</message>");  
        bsession=true;
    }
    else
    {
        strcpy(sendbuffer,"<retcode>-1</retcode><message>失败。</message>");
    }
    return true;
}   

// 查询余额业务处理函数。
bool srv002(const char *recvbuffer, char *sendbuffer)
{
    // <srvcode>2</srvcode><cardid>62620000000001</cardid>
    char cardid[31];
    GetXMLBuffer(recvbuffer, "cardid", cardid, 30);

    // 把处理结果生成strsendbuffer
    if(strcmp(cardid,"62620000000001")==0) 
    {
        strcpy(sendbuffer, "<retcode>0</retcode><message>成功。</message><ye>100.58</ye>");
    }
    else
    {
       strcpy(sendbuffer, "<retcode>-1</retcode><message>失败。</message>"); 
    }

    return true;
}

