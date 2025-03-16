//CSC60 Dylan Prosser

#include "./include/lab7.h"
int factorialValue;
void setFactorial(int x){
        int i;
        for(i = x-1; i>0; i--){
                x = x * i;
        }
      factorialValue = x; 
}
