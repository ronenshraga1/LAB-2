#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <sys/types.h>
#include <sys/wait.h>


void execute(cmdLine *pCmdLine) {
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
        if (strcmp(cmd->arguments[0], "quit") == 0) {
            freeCmdLines(cmd);
            break;
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
            if (cmd->blocking) {
                int status;
                waitpid(pid, &status, 0);
            }

        // if (!WIFEXITED(status)) {
        //     printf("pid:%d\n",pid);
        //     printf("filename:%s\n",cmd->arguments[0]);
        // }

        freeCmdLines(cmd);
        }   
    
    }
    return 0;
}
/*question 1: *execute() קורא ל־execv
execv מחליף את התהליך שלך
ה־shell שלך נעלם
רק ls רץ

👉 לכן הלולאה “נגמרת” למרות שהיא אינסופית */
/*question 2: execv doesnt look at enviroment so it tries to see file ls but it doesnt exist
execvp for example looks at env and ls will work there*/