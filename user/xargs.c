#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

#define MSGSIZE 512
//echo hello too | xargs echo bye
int main(int argc, char* argv[]){
    
    //Q1: how to get the last cmd's output
    char buf[MSGSIZE];

    //Q2: how to get itself's input
    char* xargv[MAXARG];
    int xargc = 0;

    //... | xargs echo bye
    //xargs[0] -> 'echo'
    //xargs[1] -> 'bye'
    for(int i = 1; i < argc; i++){
        xargv[xargc] = argv[i];
        xargc++;
    }

    //pointer to the start of the buf
    char* p = buf;
    int n = 0;
    while ((n = read(0, p, 1)) > 0){
        //Q3: how to use exec to execute cmd
        if(*p == '\n'){
            *p = 0;

            xargv[xargc] = buf;
            xargv[xargc+1] = 0;

            int pid = fork();
            if(pid < 0){
                fprintf(2, "Error: fork failed\n");
                exit(1);
            }
            
            
            if(pid == 0){
                //Child
                
                exec(xargv[0], xargv);
                fprintf(2, "Error: exec failed\n");
                exit(1);
                
            }else{
                //Parent
                wait(0);
            }
            p = buf;
        }else{
            p++;
        }
        
    }
    exit(0);
}

// $mkdir a
// $echo hello > a/b
// $mkdir c
// $echo hello > c/b
// $echo hello > b
// $find . b | xargs grep hello

// find . b | xargs grep hello:
// xargs will run:
// grep hello ./a/b ./c/b ./b (Which searches for the string "hello" inside each of these files.)


// find . b | grep hello
// If find outputs:
// ./a/b
// ./c/b
// ./b
// grep hello searches within these paths:
//     Does "./a/b" contain "hello"? No.
//     Does "./c/b" contain "hello"? No.
//     Does "./b" contain "hello"? No.


// Example for Clarity:
// Step-by-Step with Input hello\nWorld:
// 1. Initial Setup:
//     * p points to buf, so both p and buf are at the same location initially.
// 2. Reading hello:
//     * Reads h into buf[0], moves p to buf[1].
//     * Reads e into buf[1], moves p to buf[2].
//     * Continues until o is read into buf[4], moves p to buf[5].
// 3. Encountering Newline (\n):
//     * *p = 0 terminates the string, making buf contain "hello".
//     * Processes the command.
//     * Resets p to buf.
// 4. Reading World:
//     * p now points to buf[0] again.
//     * Reads W into buf[0], moves p to buf[1].
//     * Continues until d is read into buf[4], moves p to buf[5].
// 5. Encountering Newline (\n):
//     * *p = 0 terminates the string, making buf contain "World".
//     * Processes the command.
//     * Resets p to buf.
// By reading into p, the program can handle multiple commands in sequence and reset the buffer correctly for each new input line.