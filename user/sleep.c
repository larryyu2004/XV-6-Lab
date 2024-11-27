#include "user.h"

//obtain the command-line arguments passed to a program in the xv6
//argv[0] actually represents the name of the command itself 
//argv[1] represents what did u enter back of the 'sleep'
//argv[1] == x; $sleep [x];
//argv[2] represents NULL
//argc is index of the argv
int main(int argc, char *argv[]){

    //if user didn't enter anything behind 'sleep'
    if(argc == 1){
        printf("Error! User didn't enter anything.\n");
        exit(0);
    }

    if(argc >= 3){
        printf("Error! User entered too many arguments\n");
        exit(0);
    }


    int sleepTime = atoi(argv[argc-1]);
    printf("Sleep ... zzz \n");
    //In XV-6, sleep(100) means sleep 100 ticks
    //In XV-6, 100 ticks == 1s
    sleep(sleepTime);
    printf("Wake up!\n");
    exit(0);
}