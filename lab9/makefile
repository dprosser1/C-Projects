#Dylan Prosser
#CSC60-Spr2024
#Lab 9 Makefile

CC = gcc

lab9: lab9.o lab9.h loadData.o printCar.o features.o
	@echo Linking...
	$(CC) -o lab9 lab9.o lab9.h loadData.o printCar.o features.o

lab9.o: lab9.c lab9.h loadData.o printCar.o
	@echo Compiling...
	$(CC) -c lab9.c

loadData.o: lab9.h
	$(CC) -c loadData.c

printCar.o: lab9.h
	$(CC) -c printCar.c

features.o:
	$(CC) -c features.c

clean:
	$(RM) lab9 lab9.o loadData.o printCar.o features.o
