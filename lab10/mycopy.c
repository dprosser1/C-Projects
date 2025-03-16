#include <fcntl.h>      // needed for open
#include <sys/stat.h>   // needed for open
#include <unistd.h>     // needed for close, read, write
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUF_SIZE        /* Allow "cc -D" to override definition */
#define BUF_SIZE 1024
#endif

int main(int argc, char *argv[])
{
    int inputFd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "%s old-file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open input file */
    inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1) {
        perror("opening input file");
        exit(EXIT_FAILURE);
    }


    while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0) {
        if (write(STDERR_FILENO, buf, numRead) != numRead) {
            perror("couldn't write whole buffer to stderr");
            exit(EXIT_FAILURE);
        }
    }

    if (numRead == -1) {
        perror("read error");
        exit(EXIT_FAILURE);
    }

    // Close the input file and leave
    if (close(inputFd) == -1) {
        perror("close input");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

