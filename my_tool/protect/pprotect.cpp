#include "_public.h"

#define MAXPROC 500  //进程最大数量
#define SHMKEY 0x5056  //共享内存的key

struct st_info{
    int pid;
    char pname[51];
    int timeout;
    time_t atime;
};

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        cout<<"Using "<<argv[0]<<" pName"<<endl;
        return 0;
    }

    int shmid = -1;
    //创建共享进程 键为SHMKEY
    if(-1 == (shmid = shmget(SHMKEY, MAXPROC*sizeof(struct st_info), 0640 | IPC_CREAT)))
    {
        perror("shmid");
        return 1;
    }

    //封装的信号量处理类
    CSEM m_sem;
    if(false == m_sem.init(SHMKEY))
    {
        cout<<"m_sem.init(SHMKEY) failed"<<endl;
        return 1;
    }

    //把共享内存链接到当前进程地址空间
    struct st_info* m_shm = nullptr;   
    if((void*)-1 == (m_shm = (st_info*)shmat(shmid,0,0)))
    {
        perror("shmat");
        return 1;
    }

    //创建当前进程心跳信息结构体变量，并把本进程信息填进去
    st_info stpinfo;
    memset(&stpinfo, 0, sizeof(st_info));
    stpinfo.pid = getpid();
    STRNCPY(stpinfo.pname, sizeof(stpinfo.pname), argv[1], 50);
    stpinfo.timeout = 30;
    stpinfo.atime = time(NULL);

    int m_pos = -1;
    //如果共享内存中存在当前进程编号，一定是其他进程的残留数据，当前进程直接重置该空间
    for(int i = 0; i < MAXPROC; i++)
    {
        if(m_shm[i].pid == stpinfo.pid)
        {
            m_pos = i;
            break;
        }
    }

    m_sem.P();
    //在共享内存中查找一个空位置，把当前进程的心跳信息存入共享内存中
    if(-1 == m_pos)
    {
        for(int i = 0; i < MAXPROC; i++)
        {
            if(m_shm[i].pid == 0)
            {
                m_pos = i;
                break;
            }
        }
    }

    if(-1 == m_pos)
    {
        //异常终止要解锁
        m_sem.V();
        cout<<"共享内存已经用完"<<endl;
    }

    memcpy(&m_shm[m_pos], &stpinfo, sizeof(st_info));
    m_sem.V();

    while(true)
    {
        //更新共享内存中本进程的心跳时间
        m_shm[m_pos].atime = time(0);
        sleep(10);
    }

    //把当前进程从共享内存中移去
    //m_shm[m_pos].pid = 0;
    memset(m_shm + m_pos, 0, sizeof(st_info));

    //把共享内存从当前进程中分离开
    shmdt(m_shm);
}


