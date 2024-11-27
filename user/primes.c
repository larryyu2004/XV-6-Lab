#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define NUMSSIZE 36

void primes(int pipe_read, int startIndex){
    char nums[NUMSSIZE];

    int n = read(pipe_read, nums, NUMSSIZE);
    //read nothing
    if(n <= 0){
        close(pipe_read);
        return;
    }


    int value = -1;
    for(int i = startIndex; i < NUMSSIZE; i++){
        if(nums[i] == 'T'){
            value = i;
            printf("prime %d\n", i);
            break;
        }
    }

    //if more value were found
    if(value == -1){
        close(pipe_read);
        return;
    }

    for(int i = value*2; i < NUMSSIZE; i += value){
        nums[i] = 'F';
    }


    int out_pipe[2];
    if(pipe(out_pipe) == -1){
        printf("Error: failed to create child process");
        close(pipe_read);
        exit(1);
    }
    int pid = fork();
    if(pid == -1){
        printf("Error: failed to create process\n");
        close(pipe_read);
        close(out_pipe[0]);
        close(out_pipe[1]);
        exit(1);
    }

    //child process
    if(pid > 0){
        close(out_pipe[0]);
        write(out_pipe[1], nums, NUMSSIZE);
        close(out_pipe[1]);
        wait(0);
    }
    
    //grandson process
    if(pid == 0){
        close(out_pipe[1]);
        primes(out_pipe[0], value+1);
        close(out_pipe[0]);
        exit(0);
    }
}


int main(){

    int in_pipe[2];

    //create a pipe
    if(pipe(in_pipe) == -1){
        printf("Error: failed to create pipe\n");
        exit(-1);
    }

    //create a process
    int pid = fork();
    if(pid == -1){
        printf("Error: failed to create process");
        exit(-1);
    }
    
    //parent process
    if(pid > 0){
        close(in_pipe[0]);
        //init buf
        char nums [NUMSSIZE];
        nums[0] = 'F';
        nums[1] = 'F';
        for(int i = 2; i < NUMSSIZE; i++){
            nums[i] = 'T';
        }
        write(in_pipe[1], nums, NUMSSIZE);
        close(in_pipe[1]);
        wait(0);
    }

    //child process
    if(pid == 0){
        close(in_pipe[1]);
        primes(in_pipe[0], 2);
        close(in_pipe[0]);
        exit(0);
    }

    exit(0);
    
}