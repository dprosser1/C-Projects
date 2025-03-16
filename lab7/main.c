//CSC60 Dylan Prosser
#include <stdio.h>
#include <stdlib.h>
#include "./include/lab7.h"


int main(int argc, char* argv[]){
    
    char inChar;
    int add5ret;
    register int inputNum;
    int factorial;
    int addPrevResult;    

    if (argc < 2){
        exit(EXIT_FAILURE);
    }
    
    inChar = *argv[1];
    inputNum =  atoi(argv[1]);

    add5ret = add5(inputNum);
    printf("\n== Value from add5() is %d\n", add5ret);
        
    setFactorial(inputNum);
    printf("\n== Value from setFactorial() is %d\n", factorialValue);

    addPrevResult = addPrevious(inputNum);
    printf("\n== Value from 1st addPrevious() is %d\n", addPrevResult);

    addPrevResult = addPrevious(inputNum);
    printf("\n== Value from 2nd addPrevious() is %d\n\n", addPrevResult);

   // printf("\n Char is %c\n", inChar);
   // printf("\n inputNum is %d\n", inputNum);
}
