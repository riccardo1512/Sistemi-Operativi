#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct espandibile {

	int* array;
	size_t size;
	size_t count;
} espandibile;

// funzione che inizializza
void init(espandibile *e, size_t size) {

	e->array = malloc(size*sizeof(int));
	e->size = size;
	e->count = 0;
}

// funzione per aggiungere un elemento
void add_elem(espandibile *e, int elem) {

	if(e->count == e->size) {
		size_t new_size = 2 * e->size * sizeof(int);
		e->array = realloc(e->array, new_size);
		e->size = new_size / sizeof(int);
	}

	e->array[e->count] = elem;
	e->count++;
}

// funzione per eliminare un elemento
void del_elem(espandibile *e, int i) {

	// usiamo una memcpy
	memcpy(&(e->array[i]), e->array+i+1, (e->count - (i+1))*sizeof(int));
	e->count--;

	// se mi trovo con meno del 50% di utilizzazione allora dimezzo
	if(e->count < e->size/2) {

		e->array = realloc(e->array, (e->size/2)*sizeof(int));
		e->size = e->size/2;
	}
}


// funzione per stampare l'array
void print(espandibile *e) {

	printf("Capacità: %ld\n", e->size);
	printf("Occupati: %ld\n", e->count);

	for(int i = 0; i < e->count; i++) {

		printf("array[%d] = %d\n", i, e->array[i]);
	}

	printf("\n");
}


int main(int argc, char* argv[]) {

	espandibile e;
	size_t size = atol(argv[1]);
	init(&e, size);

	printf("Stampa con struttura vuota:\n");
	print(&e);

	add_elem(&e, 10);
	add_elem(&e, 20);
	add_elem(&e, 30);

	printf("Stampa con 3 elementi:\n");
	print(&e);

	del_elem(&e, 0);
	del_elem(&e, 1);
	print(&e);
}
