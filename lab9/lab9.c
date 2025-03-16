#include <stdio.h>
#include <stdlib.h>
#include "lab9.h"

int main() {
    car_t* head = loadData();
    car_t* current = head;

    while (current != NULL) {
        printCar(current);
        current = current->next;
    }


    current = head;
    while (current != NULL) {
        car_t* next = current->next;
        free(current);
        current = next;
    }

    exit(EXIT_SUCCESS);
}

