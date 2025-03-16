#include <stdio.h>
#include <stdlib.h>

#define PS_MASK 0x1      // Has Power Steering
#define PB_MASK 0x2      // Has Power Brakes
#define AT_MASK 0x4      // Has Automatic Transmission
#define AC_MASK 0x8      // Has Air Conditioning
#define ST_MASK 0x10     // Has a Soft Top (convertible)


// Define the structure for the car
typedef struct car {
    unsigned int carID;
    char make[20];
    int year;
    char color[20];
    unsigned int features;
    struct car* next;
} car_t;

// Function prototypes
car_t* loadData();
void printCar(car_t* car);
int carHasFeature(car_t thisCar, unsigned int featureMask);
