#include<iostream>
#include<cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include<cstdio>

class myclass{
public:
    pid_t pid;
    char name[51];
};

int main(int argc, char** argv)
{
    int shmid = -1;
    //创建共享进程 键为
    if(-1 == (shmid = shmget(0x08, sizeof(myclass), 0644 | IPC_CREAT)))
    {
        perror("shmid");
        return 1;
    }

    myclass* ptr = nullptr;

    if((void*)-1 == (ptr = (myclass*)shmat(shmid,0,0)))
    {
        perror("shmat");
        return 1;
    }

    std::cout<<sizeof(myclass)<<std::endl;

    std::cout<<ptr->pid<<" "<<ptr->name<<std::endl;
    ptr->pid = getpid();
    strcpy(ptr->name, argv[1]);
    std::cout<<ptr->pid<<" "<<ptr->name<<std::endl;
    
    shmdt(ptr);


}