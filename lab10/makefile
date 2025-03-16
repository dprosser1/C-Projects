#CSC60 SPRING 2024
#Dylan Prosser
#Lab 10

CC = gcc

all: lab10 mycopy

lab10: lab10.o
	@echo linking lab10...
	$(CC) -o lab10 lab10.o

lab10.o: lab10.c lab10.h
	@echo compiling lab10...
	$(CC) -c lab10.c

mycopy: mycopy.o
	@echo linking mycopy...
	$(CC) -o mycopy mycopy.o

mycopy.o: mycopy.c
	@echo compiling mycopy...
	$(CC) -c mycopy.c

clean:
	$(RM) lab10 lab10.o mycopy mycopy.o
