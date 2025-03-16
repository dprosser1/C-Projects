#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sigusr1_handler(int signo) {
    printf("Process %d caught SIGUSR1\n", getpid());
}

int main() {
    signal(SIGUSR1, sigusr1_handler);

    while (1) {
        sleep(1);
    }

    return 0;
}

