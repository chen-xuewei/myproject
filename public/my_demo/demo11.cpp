/*
 * 程序名：demo11.cpp，此程序用于演示网银APP软件的客户端，增加了心跳报文
*/
#include "../_public.h"

#define BUFSIZE 1024

CTcpClient tcpClient;
bool srv000();
bool srv001();    // 登录业务。
bool srv002();    // 我的账户（查询余额）。

int main(int argc,char *argv[])
{
    if (argc!=3)
    {
        printf("Usage:./TcpClient <ip> <port>\n"); 
        return -1;
    }

    //向服务端发起连接请求
    if(!tcpClient.ConnectToServer(argv[1], atoi(argv[2])))
    {
        printf("tcpClient.ConnectToServer(%s,%s) failed.\n",argv[1],argv[2]); 
        return -1;
    }

    //登录业务
    if(!srv001())
    {
        std::cout<<"srv001() failed."<<std::endl;
        return -1;
    }

    //模拟心跳报文
    for(int i = 1; i < 5; i++)
    {
        if(!srv000()) break;
        sleep(8+i);
    }

    //查询余额
    if(!srv002())
    {
        std::cout<<"srv002() failed."<<std::endl;
        return -1;
    }
}

// 心跳。 
bool srv000()    
{
  char buffer[1024];
  SPRINTF(buffer,sizeof(buffer),"<srvcode>0</srvcode>");
  printf("发送：%s\n",buffer);
  if (tcpClient.Write(buffer)==false) return false; // 向服务端发送请求报文。

  memset(buffer,0,sizeof(buffer));
  if (tcpClient.Read(buffer)==false) return false; // 接收服务端的回应报文。
  printf("接收：%s\n",buffer);

  return true;
}

bool srv001()
{
    char buffer[BUFSIZE];
    SPRINTF(buffer,sizeof(buffer),"<srvcode>1</srvcode><tel>18581765402</tel><password>123456</password>");
    if(!tcpClient.Write(buffer))
    {
        return false;;
    }
    printf("发送：%s\n",buffer);

    memset(buffer,0,sizeof(buffer));
    if (!tcpClient.Read(buffer)) return false; // 接收服务端的回应报文。
    printf("接收： %s\n", buffer);

    //解析服务端返回的xml
    int iretcode = -1;
    GetXMLBuffer(buffer, "retcode", &iretcode);
    if (iretcode!=0) 
    {   
        printf("登录失败。\n"); 
        return false; 
    }

    std::cout<<"登录成功"<<std::endl;
    return true;
}

// 我的账户（查询余额）。
bool srv002()
{
    char buffer[BUFSIZE];
    SPRINTF(buffer, BUFSIZE, "<srvcode>2</srvcode><cardid>62620000000001</cardid>");
    if(!tcpClient.Write(buffer)) return false;
    printf("发送：%s\n",buffer);

    memset(buffer, 0, BUFSIZE);
    if (!tcpClient.Read(buffer)) return false;
    printf("发送：%s\n",buffer);

    //解析服务端返回的xml
    int iretcode = -1;
    GetXMLBuffer(buffer, "retcode", &iretcode);
    if(iretcode != 0)
    {
        printf("登录失败。\n"); 
        return false; 
    }

    double ye = 0;
    GetXMLBuffer(buffer,"ye",&ye);
    printf("查询余额成功(%.2f)。\n",ye);
    return true;
}