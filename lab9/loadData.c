#include <stdio.h>
#include <stdlib.h>
#include "lab9.h"

#define IN_FILE_NAME "cars.dat"

car_t* loadData() {
    FILE *infile;
    infile = fopen(IN_FILE_NAME, "r");

    if (infile == NULL) {
        printf("Error Opening input file");
        exit(EXIT_FAILURE);
    }

    car_t* head = NULL;
    car_t* current = NULL;
    car_t* new_car = NULL;

    while (1) {
        new_car = (car_t*)malloc(sizeof(car_t));

        if (fscanf(infile, "%u%s%i%s", &new_car->carID, new_car->make, &new_car->year, new_car->color) != 4) {
            free(new_car);
            break;
        }

        new_car->next = NULL;

        if (head == NULL) {
            head = new_car;
        } else {
            current->next = new_car;
        }

        current = new_car;
    }

    fclose(infile);
    return head;
}

