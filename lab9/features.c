#include "lab9.h"
int carHasFeature(struct car thisCar, unsigned int featureMask) {
    return (thisCar.features & featureMask) != 0;
}

