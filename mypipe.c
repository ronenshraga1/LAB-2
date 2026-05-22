#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIZE 2048

int main(int argc,char **argv){
    char buffer[SIZE];
    int p[2];// writing in p[1] , reading in p[0]
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        return 1;
    }

    if (pipe(p) < 0){
        perror("pipe error");
        return 1;
    }
        
    int pid = fork();

    if(pid == 0){
        close(p[1]);
        ssize_t n = read(p[0], buffer, SIZE - 1);
        if (n < 0) {
            perror("read");
            _exit(1);
        }

        buffer[n] = '\0';
        printf("%s\n", buffer);

        close(p[0]);
        _exit(0);
    } else{
        close(p[0]);
        write(p[1], argv[1], strlen(argv[1]));
        close(p[1]);
        waitpid(pid, NULL, 0);

    }
    return 0;
}