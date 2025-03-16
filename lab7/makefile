#CSC60-02 Spring2024
#Dylan Prosser
#Lab 7

CC = gcc

lab7: main.o add5.o addPrevious.o setFactorial.o
	@echo "Linking..."
	$(CC) -o lab7 main.o add5.o addPrevious.o setFactorial.o 
	
main.o: ./include/lab7.h
	@echo "Compiling..."
	$(CC) -c main.c
	
add5.o: add5.c ./include/lab7.h
	$(CC) -c add5.c 

setFactorial.o: setFactorial.c ./include/lab7.h
	$(CC) -c setFactorial.c

addPrevious.o: addPrevious.c ./include/lab7.h
	$(CC) -c addPrevious.c

clean:
	$(RM) main.o add5.o addPrevious.o setFactorial.o lab7
	
