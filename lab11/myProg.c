#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lab11.h"

int main(int argc, char *argv[]) {
    int numLoops;

    if (argc < 2) {
	numLoops = NUMCHILDLOOPS;
    } else {
	numLoops = atoi(argv[1]);
    }

    printf("\tHere in %s (PID=%d), Looping %d times\n",
	argv[0], getpid(), numLoops);

    for (int i=0; i<numLoops; i++) {
	printf("\t\tworking...%d\n", i+1);
	sleep(NUMSECS);
    }
}
