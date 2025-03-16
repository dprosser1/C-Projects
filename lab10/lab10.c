//Dylan Prosser Lab10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For environ
#include "lab10.h"

int main(int argc, char *argv[], char *envp[]) {


    char *yoyo_value_method1 = NULL;
    for (int i = 0; envp[i] != NULL; i++) {
        if (strncmp(envp[i], YOYO "=", strlen(YOYO) + 1) == 0) {
            yoyo_value_method1 = envp[i] + strlen(YOYO) + 1; // Skip "YOYO="
            break;
        }
    }


    if (yoyo_value_method1 != NULL) {
        printf("Method 1: YOYO=%s\n", yoyo_value_method1);
    } else {
        printf("Method 1: YOYO environment variable not found\n");
    }



    extern char **environ;


    char *yoyo_value_method2 = NULL;
    for (int i = 0; environ[i] != NULL; i++) {
        if (strncmp(environ[i], YOYO "=", strlen(YOYO) + 1) == 0) {
            yoyo_value_method2 = environ[i] + strlen(YOYO) + 1; // Skip "YOYO="
            break;
        }
    }


    if (yoyo_value_method2 != NULL) {
        printf("Method 2: YOYO=%s\n", yoyo_value_method2);
    } else {
        printf("Method 2: YOYO environment variable not found\n");
    }



    char *yoyo_value_method3 = getenv(YOYO);


    if (yoyo_value_method3 != NULL) {
        printf("Method 3: YOYO=%s\n", yoyo_value_method3);
    } else {
        printf("Method 3: YOYO environment variable not found\n");
    }

    return 0;
}

