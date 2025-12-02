#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 30 min

typedef struct espandibile {

    int* array;
    int size;
    int count;
} espandibile;

void init(espandibile* e) {

    e->array = malloc(2*sizeof(int));
    e->size = 2;
    e->count = 0;       // elemento che posso scrivere
}
// size = 2
// count = 0
// 0 1

int add_elem(espandibile* e, int num) {

    // size non sufficiente
    if(e->count >= e->size) {

        e->size *= 2;
        e->array = realloc(e->array, e->size);
        e->array[e->count] = num;
        e->count++;
        return 1;
    }
    else {

        e->array[e->count] = num;
        e->count++;
        return 1;
    }

    return 0;       // non ho aggiunto nulla
}

// 0 1 2 3 4 5

void remove_elem(espandibile* e, int i) {

    size_t numero_byte_da_copiare = (e->count-i-1)*sizeof(int);

    memcpy(&(e->array[i]), &(e->array[i+1]),numero_byte_da_copiare);
    e->count--;
    //e->size = e->count;
}

void print(espandibile* e) {

    printf("\nprint:\n");

    for(int i = 0; i < e->count; i++) {

        printf("espandibile[%d] = %d\n", i, e->array[i]);
    }
}

void print_total(espandibile* e) {

    printf("\nprint_total:\n");

    for(int i = 0; i < e->size; i++) {

        printf("espandibile[%d] = %d\n", i, e->array[i]);
    }
}

int main() {

    espandibile e;

    init(&e);

    print_total(&e);
    print(&e);

    add_elem(&e, 0);
    add_elem(&e, 10);
    add_elem(&e, 20);
    add_elem(&e, 30);
    add_elem(&e, 40);
    add_elem(&e, 50);

    print_total(&e);
    print(&e);

    remove_elem(&e, 3);

    print_total(&e);
    print(&e);

    return 0;
}