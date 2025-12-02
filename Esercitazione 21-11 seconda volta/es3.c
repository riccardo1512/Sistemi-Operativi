#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 11 min 45 sec

void print_array(int* arr, size_t size) {

    printf("\n");

    for(int i = 0; i < (int) size; i++) {

        printf("array[%d] = %d\n", i, arr[i]);
    }

    printf("\n");
}

int main(int argc, char* argv[]) {


    size_t size = 5;

    int* array1 = malloc(((int) size)*sizeof(int));
    printf("\nStampa array1:");
    print_array(array1, size);

    memset(array1, 0, size);
    printf("\nStampa array1 dopo memset:");
    print_array(array1, size);

    
    int* array2 = malloc(((int) size)*sizeof(int));
    array2[0] = 0;
    array2[1] = 10;
    array2[2] = 20;
    array2[3] = 30;
    array2[4] = 40;
    printf("\nStampa array2:");
    print_array(array2, size);


    memcpy(array1, array2, size*sizeof(int));
    printf("\nStampa array1 dopo memcpy:");
    print_array(array1, size);

    return 0;
}