#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) {
        printf("Here in lab11 child: PID is %d\n", getpid());
        fflush(stdout);
        char *program = argv[1];
        char *args[] = {program, NULL};
        execvp(program, args);
        perror("Execution error");
        fprintf(stderr, "Only gets here on error\n");
        return 1;
    } else {
        printf("Here in lab11 parent: PID is %d\n", getpid());
        fflush(stdout);
        sleep(3);
        kill(pid, SIGUSR1);
        wait(NULL); 
    }

    return 0;
}

