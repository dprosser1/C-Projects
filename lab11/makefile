CC = gcc

all: lab11 lab11_fork lab11_signal myProg myProg_signal

lab11: lab11.c
	@echo Linking...
	$(CC) -o lab11 lab11.c

lab11_fork: lab11_fork.c
	@echo Linking...
	$(CC) -o lab11_fork lab11_fork.c

lab11_signal: lab11_signal.c
	@echo Linking...
	$(CC) -o lab11_signal lab11_signal.c

myProg: myProg.c lab11.h
	@echo Linking...
	$(CC) -o myProg myProg.c

myProg_signal: myProg_signal.c lab11.h
	@echo Linking...
	$(CC) -o myProg_signal myProg_signal.c

clean:
	@echo Cleaning...
	$(RM) lab11 lab11.o lab11_fork lab11_signal myProg myProg_signal

