#include <stdio.h>
#include <stdlib.h>

// 8 min 30 sec

int main(int argc, char* argv[]) {

    if(argc != 2) {

        fprintf(stderr, "Errore con i parametri specificati\n");
        exit(1);
    }

    size_t dim = atol(argv[1]);

    int* array = malloc(sizeof(int) * dim);

    for(int i = 0; i < dim; i++) {

        array[i] = (int) random();
    }

    for(int i = 0; i < dim; i++) {

        printf("array[%d] = %d\n", i, array[i]);
    }

    free(array);

    return 0;
}