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
        
    int pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if(pid == 0){
        dup2(p[1], STDOUT_FILENO);//dup2 also closes STDOUT_FILENO
        close(p[1]);
        close(p[0]);
        char *args[] = {"ps", "-xl", NULL};
        execvp("ps", args);        
        perror("execvp failed");
        _exit(1);
    }
    close(p[1]);
    int pid2=fork();
    if (pid2 < 0) {
        perror("fork failed");
        return 1;
    }

    if(pid2==0){
        dup2(p[0],STDIN_FILENO);
        close(p[0]);
        char *args[] = {"grep","5",NULL};
        execvp("grep",args);
        perror("execvp failed");
        _exit(1);
    }
    close(p[0]);
    waitpid(pid,NULL,0);
    waitpid(pid2,NULL,0);
    }

