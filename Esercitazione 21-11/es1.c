#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

	size_t size;
	if(argc != 2) {

		fprintf(stderr, "Sintassi: es1 <dimensione>\n");
		exit(1);
	}

	size = atol(argv[1]);

	int* array = malloc(size * sizeof(int));


	for(int i = 0; i < size; i++) {

		array[i] = (int) random();
	}

	for(int i  = 0; i < size; i++) {

		printf("array[%d] = %d\n", i, array[i]);
	}

	free(array);

	return 0;
}
