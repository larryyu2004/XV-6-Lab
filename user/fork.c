#include "kernel/types.h"
#include "user/user.h"

int main(){
    int pid = fork();

    //Parent and child process will run at same time
    
    //child
    if(pid == 0){
        sleep(1);
        printf("fork() returned %d\n", pid);
        printf("Child\n");
        
    //parent
    }else{
        printf("fork() returned %d\n", pid);
        printf("Parent\n");
        printf("\n");
    }
    exit(0);
}