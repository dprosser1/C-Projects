#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program>\n", argv[0]);
        return 1;
    }

    printf("Here in lab11: PID is %d\n", getpid());
    fflush(stdout);

    char *program = argv[1];
    char *args[] = {program, NULL};

    if (execvp(program, args) == -1) {
        perror("Execution error");
        fprintf(stderr, "Only gets here on error\n");
        return 1;
    }

    return 0;
}

