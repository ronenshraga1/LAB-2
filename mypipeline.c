#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define SIZE 2048

int main(int argc,char **argv){
    int p[2];// writing in p[1] , reading in p[0]
    if (pipe(p) < 0){
        perror("pipe error");
        return 1;
    }
    fprintf(stderr, "(parent_process>forking…)\n");    
    int pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if(pid == 0){
        dup2(p[1], STDOUT_FILENO);//dup2 also closes STDOUT_FILENO
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");
        close(p[1]);
        close(p[0]);
        char *args[] = {"ps", "-xl", NULL};
        fprintf(stderr, "(child1>going to execute cmd: …)\n");
        execvp("ps", args);        
        perror("execvp failed");
        _exit(1);
    } else{
        fprintf(stderr, "(parent_process>created process with id: %i)\n",pid); 
    }
    fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n"); 
    close(p[1]);
    int pid2=fork();
    if (pid2 < 0) {
        perror("fork failed");
        return 1;
    }

    if(pid2==0){
        dup2(p[0],STDIN_FILENO);
        fprintf(stderr, "(child2>redirecting stdin  to the read end of the pipe…)\n");
        close(p[0]);
        char *args[] = {"grep","5",NULL};
        fprintf(stderr, "(child2>going to execute cmd: …)\n");
        execvp("grep",args);
        perror("execvp failed");
        _exit(1);
    } else{
        fprintf(stderr, "(parent_process>created process with id: %i)\n", pid2);
    }
    fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n"); 
    close(p[0]);
    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n"); 
    waitpid(pid,NULL,0);
    waitpid(pid2,NULL,0);
    fprintf(stderr, "(parent_process>exiting…)\n");
    return 0;
    }
/*1. in the first case the program wait for user input'
 and does not receive eof because we removed closep[1] , it continues to wait for because it think we still writing. 
 2.works because we closed write on child and parent,EOF depnedent on that write ends will close then it still works,no recom.resource leak
 */
