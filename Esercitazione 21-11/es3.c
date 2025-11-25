#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {

	size_t size;
	if(argc != 2) {

		fprintf(stderr, "Sintassi: es1 <dimensione>\n");
		exit(1);
	}

	size = atol(argv[1]);

	int* array = malloc(size * sizeof(int));

	// riempio l'array con numeri random
	for(int i = 0; i < size; i++) {

		array[i] = (int) random();
	}

	int *array2 = malloc(size* sizeof(int)); //sempre sull'heap, big
	memset(array2, 0, size*sizeof(int));	// metto tutti 0
	// stampo l'array
	for(int i = 0; i < size; i++) {

		printf("%d\n", array2[i]);
	}

	memcpy(array2, array, size*sizeof(int));

	for(int i = 0; i < size; i++) {

		printf("array[%d]=%d\n", i, array[i]);
		printf("array2[%d])=%d\n", i, array2[i]);
	}

	free(array);
	free(array2);

	return 0;
}
