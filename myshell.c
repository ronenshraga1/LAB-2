#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include "LineParser.h"
void execute(cmdLine *pCmdLine) {
    if(execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
        perror("execv failed");
        freeCmdLines(pCmdLine);
        exit(1);
    }
    freeCmdLines(pCmdLine);
    exit(0);

}
int main(int argc,char**argv){
    char cwd[PATH_MAX];
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
        execute(cmd);

       freeCmdLines(cmd);
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