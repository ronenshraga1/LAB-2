#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

void Redirect(cmdLine *cmd){
    if(cmd->inputRedirect){
    int fd = open(cmd->inputRedirect,O_RDONLY);
    if (fd < 0) {
        perror("open output failed");
        _exit(1);
    }
    dup2(fd,0);// 0 for stdin, dup 2 now refers new fd to old one
    close(fd);
    }
    if(cmd->outputRedirect){
        int fd = open(cmd->outputRedirect,O_WRONLY |O_CREAT|O_TRUNC,0644); // 0644 is like: 06 owner r&w, group:r,other:r.https://stackoverflow.com/questions/18415904/what-does-mode-t-0644-mean
        if (fd < 0) {
            perror("open output failed");
            _exit(1);
        }
        dup2(fd,1);
        close(fd);
    }
}

void execute(cmdLine *pCmdLine) {
    Redirect(pCmdLine);
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);
    perror("execvp failed");
    _exit(1);// not regular exit because we use fork and parent and child use same buffers so it could destroy his buffer.
}
int main(int argc,char**argv){
    char cwd[PATH_MAX];
    int debug = 0;
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        debug = 1;
    }
    while(1){
        if(getcwd(cwd,sizeof(cwd))!=NULL){
            printf("current directory:%s\n",cwd);
        } else{
            perror("getcwd failed");
        }
        char buffer[2048];
        fgets(buffer,sizeof(buffer),stdin);
       cmdLine* cmd = parseCmdLines(buffer);
       if (cmd == NULL)
            continue;
        if (strcmp(cmd->arguments[0], "cd") == 0) {
            if (cmd->argCount < 2) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (chdir(cmd->arguments[1]) == -1) {
                    perror("cd failed");
                }
            }
            freeCmdLines(cmd);
            continue;
        }
        

        if (strcmp(cmd->arguments[0], "quit") == 0) {
            freeCmdLines(cmd);
            break;
        }
        if(strcmp(cmd->arguments[0], "stop") == 0){//kill function used to send signal to any process or process group.
          kill(atoi(cmd->arguments[1]),SIGSTOP);//atoi to make the arg a number,source:https://www.geeksforgeeks.org/c/c-atoi-function/ 
          freeCmdLines(cmd);
          continue;
        }
        if(strcmp(cmd->arguments[0], "wakeup") == 0){
          kill(atoi(cmd->arguments[1]),SIGCONT);
          freeCmdLines(cmd);
          continue;
        }
        if(strcmp(cmd->arguments[0], "ice") == 0){
          kill(atoi(cmd->arguments[1]),SIGINT);
          freeCmdLines(cmd);
          continue;
        }
        if(strcmp(cmd->arguments[0], "nuke") == 0){
          kill(-atoi(cmd->arguments[1]),SIGKILL);// - for process group
          freeCmdLines(cmd);
          continue;
        }


        int pid = fork();
        if(pid == 0){
            execute(cmd);
        } else{
            if (debug) {
                fprintf(stderr, "PID: %d\n", pid);
                fprintf(stderr, "Executing command: %s\n", cmd->arguments[0]);
                fprintf(stderr, "Mode: %s\n", cmd->blocking ? "foreground" : "background");
            }
            if (cmd->blocking) {// 1 for waiting 0 means continue
                int status;
                waitpid(pid, &status, 0);
            }

        freeCmdLines(cmd);
        }   
    
    }
    return 0;
}
