#include <stdio.h>
#include "lab9.h"

void printCar(car_t* car) {
    printf("Car: %u, %d, %s, %s", car->carID, car->year, car->make, car->color);


    if (carHasFeature(*car, AT_MASK)) {
        printf(", AT");
    }


    if (carHasFeature(*car, AC_MASK)) {
        printf(", AC");
    }

    printf("\n");
}







