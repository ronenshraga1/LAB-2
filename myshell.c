#define _GNU_SOURCE
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
#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;
#define HISTLEN 10
typedef struct historyEntry {
    int number;
    char* line;
} historyEntry;
// Defining the Queue structure
typedef struct historyQueue 
{
    historyEntry entries[HISTLEN];
    int start;
    int size;
    int nextNumber;
} historyQueue;

void initHistory(historyQueue* h) {
    h->start = 0;
    h->size = 0;
    h->nextNumber = 1;

    for (int i = 0; i < HISTLEN; i++) {
        h->entries[i].line = NULL;
        h->entries[i].number = 0;
    }
}

char* copyString(const char* s) {
    char* copy = malloc(strlen(s) + 1);
    if (copy == NULL) {
        perror("malloc failed");
        return NULL;
    }
    strcpy(copy, s);
    return copy;
}

void addHistory(historyQueue* h, const char* line) {
    int index;

    if (h->size < HISTLEN) {
        index = (h->start + h->size) % HISTLEN;
        h->size++;
    } else {
        index = h->start;
        free(h->entries[index].line);
        h->start = (h->start + 1) % HISTLEN;
    }

    h->entries[index].number = h->nextNumber++;
    h->entries[index].line = copyString(line);
}

void printHistory(historyQueue* h) {
    for (int i = 0; i < h->size; i++) {
        int index = (h->start + i) % HISTLEN;
        printf("%d %s", h->entries[index].number, h->entries[index].line);
    }
}

char* getLastHistory(historyQueue* h) {
    if (h->size == 0) {
        return NULL;
    }

    int index = (h->start + h->size - 1) % HISTLEN;
    return h->entries[index].line;
}

char* getHistoryByNumber(historyQueue* h, int number) {
    for (int i = 0; i < h->size; i++) {
        int index = (h->start + i) % HISTLEN;

        if (h->entries[index].number == number) {
            return h->entries[index].line;
        }
    }

    return NULL;
}

void freeHistory(historyQueue* h) {
    for (int i = 0; i < h->size; i++) {
        int index = (h->start + i) % HISTLEN;
        free(h->entries[index].line);
    }
}    
void freeProcessList(process* process_list){
    process* curr = process_list;

    while (curr != NULL) {
        process* next = curr->next;
        freeCmdLines(curr->cmd);
        free(curr);
        curr = next;
    }
}
void updateProcessStatus(process* process_list, int pid, int status){    
    process* curr = process_list;
    while (curr != NULL) {
        if (curr->pid == pid) {
            curr->status = status;
            return;
        }
        curr = curr->next;
    }

}

void updateProcessList(process **process_list){
    process *curr = *process_list;
    while(curr != NULL){
        int status;
        pid_t result = waitpid(curr->pid,&status,WNOHANG | WUNTRACED | WCONTINUED );
        if (result == 0) {
            // no status change
            curr = curr->next;
            continue;

        }
        else if (result == -1) {
            // maybe process already waited for, or error
            curr = curr->next;
            continue;
        }
        else{
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                updateProcessStatus(*process_list,result,TERMINATED);
            } else if(WIFSTOPPED(status)){
                updateProcessStatus(*process_list,result,SUSPENDED);
            } else if(WIFCONTINUED(status)){
                updateProcessStatus(*process_list,result,RUNNING);
            }
        }
        curr = curr->next;
    }
}

void deleteDeadProcess(process** process_list){
    process *curr = *process_list;
    process *prev = NULL;
    while (curr != NULL) {
        if (curr->status == TERMINATED) {
            process* to_delete = curr;

            if (prev == NULL) {
                *process_list = curr->next;
                curr = *process_list;
            } else {
                prev->next = curr->next;
                curr = curr->next;
            }

            freeCmdLines(to_delete->cmd);
            free(to_delete);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}
void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* new_process = malloc(sizeof(process));
    if (new_process == NULL) {
        perror("malloc failed");
        return;
    }
    new_process->cmd = cmd;
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;

    *process_list = new_process;
}
void printProcessList(process** process_list){
    updateProcessList(process_list);
    process* curr = *process_list;

    printf("PID\t\tSTATUS\t\tCommand\n");

    while (curr != NULL) {
        char* status_str;

        if (curr->status == RUNNING)
            status_str = "Running";
        else if (curr->status == SUSPENDED)
            status_str = "Suspended";
        else
            status_str = "Terminated";

        printf("%d\t\t%s\t\t", curr->pid, status_str);

        for (int i = 0; i < curr->cmd->argCount; i++) {
            printf("%s ", curr->cmd->arguments[i]);
        }

        printf("\n");

        curr = curr->next;    
  }
  deleteDeadProcess(process_list);      
}


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
int pipe2child(cmdLine* cmd){
    if (cmd->outputRedirect != NULL) {
        fprintf(stderr, "error: output redirection on left side of pipe\n");
        return 1;
    }
    if (cmd->next->next != NULL) {
        fprintf(stderr, "error: only one pipe is supported\n");
        return 1;
    }

    if (cmd->next->inputRedirect != NULL) {
        fprintf(stderr, "error: input redirection on right side of pipe\n");
        return 1;
    }
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
        Redirect(cmd);
        execvp(cmd->arguments[0], cmd->arguments);        
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
        Redirect(cmd->next);
        execvp(cmd->next->arguments[0],cmd->next->arguments);
        perror("execvp failed");
        _exit(1);
    }
    close(p[0]);
    waitpid(pid,NULL,0);
    waitpid(pid2,NULL,0);
    return 0;

}

void execute(cmdLine *pCmdLine) {
    Redirect(pCmdLine);
    execvp(pCmdLine->arguments[0], pCmdLine->arguments);
    perror("execvp failed");
    _exit(1);// not regular exit because we use fork and parent and child use same buffers so it could destroy his buffer.
}
int main(int argc,char**argv){
    char cwd[PATH_MAX];
    process* process_list = NULL;
    historyQueue history;
    initHistory(&history);
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
        if(fgets(buffer,sizeof(buffer),stdin) == NULL){
            break;
        }
        if (strcmp(buffer, "\n") == 0)
            continue;

        char lineToExecute[2048];

        if (strcmp(buffer, "!!\n") == 0) {
            char* last = getLastHistory(&history);

            if (last == NULL) {
                printf("No commands in history\n");
                continue;
            }

            printf("%s", last);
            strcpy(lineToExecute, last);
        }
        else if (buffer[0] == '!' && buffer[1] != '\0' && buffer[1] != '\n') {
            int number = atoi(buffer + 1);
            char* selected = getHistoryByNumber(&history, number);

            if (selected == NULL) {
                printf("No such command in history\n");
                continue;
            }

            printf("%s", selected);
            strcpy(lineToExecute, selected);
        }
        else {
            strcpy(lineToExecute, buffer);
        }

        addHistory(&history, lineToExecute);
       cmdLine* cmd = parseCmdLines(lineToExecute);
       if (cmd == NULL)
            continue;
        if(cmd->next !=NULL){
            pipe2child(cmd);
            freeCmdLines(cmd);
            continue;
        }
        if (strcmp(cmd->arguments[0], "history") == 0) {
            printHistory(&history);
            freeCmdLines(cmd);
            continue;
        }
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
        
        if (strcmp(cmd->arguments[0], "procs") == 0) {
            printProcessList(&process_list);
            freeCmdLines(cmd);
            continue;
        }
        if (strcmp(cmd->arguments[0], "quit") == 0) {
            freeCmdLines(cmd);
            freeProcessList(process_list);
            freeHistory(&history);
            break;
        }
        if(strcmp(cmd->arguments[0], "stop") == 0){//kill function used to send signal to any process or process group
            if(cmd->argCount < 2) { fprintf(stderr, "stop: missing argument\n"); freeCmdLines(cmd); continue; }
         if(kill(atoi(cmd->arguments[1]),SIGSTOP)== -1){//atoi to make the arg a number,source:https://www.geeksforgeeks.org/c/c-atoi-function/ 
            perror("stop failed");
          } else{
            updateProcessStatus(process_list, atoi(cmd->arguments[1]), SUSPENDED);
          }
            
          freeCmdLines(cmd);
          continue;
        }
        if(strcmp(cmd->arguments[0], "wakeup") == 0){
            if(cmd->argCount < 2) { fprintf(stderr, "wakeup: missing argument\n"); freeCmdLines(cmd); continue; }
        if(kill(atoi(cmd->arguments[1]),SIGCONT)== -1){
            perror("wakeup failed");
          } else {
            updateProcessStatus(process_list, atoi(cmd->arguments[1]), RUNNING);
        }


          freeCmdLines(cmd);
          continue;
        }
        if(strcmp(cmd->arguments[0], "ice") == 0){
            if(cmd->argCount < 2) { fprintf(stderr, "ice: missing argument\n"); freeCmdLines(cmd); continue; }
          if(kill(atoi(cmd->arguments[1]),SIGINT)== -1){
            perror("ice failed");
          } else {
            updateProcessStatus(process_list, atoi(cmd->arguments[1]), TERMINATED);
          }

          freeCmdLines(cmd);
          continue;
        }
        if(strcmp(cmd->arguments[0], "nuke") == 0){
            if(cmd->argCount < 2) { fprintf(stderr, "nuke: missing argument\n"); freeCmdLines(cmd); continue; }
          if(kill(-atoi(cmd->arguments[1]),SIGKILL)== -1){
            perror("nuke failed");
          };// - for process group
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
            addProcess(&process_list, cmd, pid);
            if (cmd->blocking) {// 1 for waiting 0 means continue
                int status;
                waitpid(pid, &status, 0);
                updateProcessStatus(process_list, pid, TERMINATED);
            }

        }   
    
    }
    return 0;
}
