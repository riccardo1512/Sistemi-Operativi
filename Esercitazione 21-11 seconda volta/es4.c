#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

// 1h 20 min

typedef struct node {

    int free;
    size_t size;
    struct node* next;

} node;

node* first;

void print_memory() {

    printf("------------Stampa della memoria:------------\n");

    node* cur = first;
    for(int i = 0; cur != NULL; i++) {

        printf("Nodo %d:\n", i);
        printf("Nodo->free = %d:\n", cur->free);
        printf("Nodo->size = %ld:\n\n", cur->size);

        cur = cur->next;
    }

    printf("------------Fine stampa memoria------------\n");
}

void* my_malloc(size_t size) {

    for(node* cur = first; cur != NULL; cur=cur->next) {

        if(cur->free && cur->size > size + sizeof(node)) {  // ho trovato il nodo che va bene

            void* pos_node = (void*) cur;
            
            node* new_node = (node*) (pos_node + sizeof(node) + size);
            new_node->free = 1;
            new_node->next = NULL;
            new_node->size = cur->size - size - sizeof(node);

            cur->next = new_node;
            cur->free = 0;
            cur->size = size;

            void* output = pos_node + sizeof(node);
            return output;
        }
    }

    return NULL;
}

void my_free(void* ptr) {

    if(ptr == NULL) {

        fprintf(stderr, "Invalid free\n");
        exit(1);
    }

    node* cur = (node*) (ptr - sizeof(node));
    cur->free = 1;
}

void merge_memory() {

  
    /* Scorro tutta la memoria */
    for(node* cur = first; cur != NULL;) {

        int mergiato = 0;

        /* se ho ancora un successivo*/
        if(cur->next != NULL) {

            /* Se ho due nodi consecutivi liberi */
            if(cur->free && cur->next->free) {

                cur->size = cur->size + sizeof(node) + cur->next->size;
                cur->next = cur->next->next;
                mergiato = 1;
            }
        }

        if(!mergiato) {

            cur = cur->next;
        }
    }
}

int main(int argc, char* argv[]) {

    if(argc != 2) {

        fprintf(stderr, "Sintassi: ./my_malloc size\n");
        exit(1);
    }

    printf("\nDimensione del nodo = %ld\n", sizeof(node));
    printf("Dimensione di int = %ld\n\n", sizeof(int));
    
    size_t dim = atol(argv[1]);    
    void* start = mmap(NULL, dim, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    first = (node*) start;
    first->size = (dim - sizeof(node));
    first->free = 1;
    first->next = NULL;

    print_memory();


    int* array = my_malloc(5*sizeof(int));
    for(int i = 0; i < 5; i++) {

        array[i] = i;
    }
    for(int i = 0; i < 5; i++) {

        printf("array[%d] = %d\n", i, array[i]);
    }

    print_memory();

    int* array2 = (int*) my_malloc(3*sizeof(int));
    int* array3 = (int*) my_malloc(3*sizeof(int));
    int* array4 = (int*) my_malloc(3*sizeof(int));

    print_memory();

    my_free(array3);
    my_free(array4);

    print_memory();

    merge_memory();
    printf("\nStampa dopo il merge\n");
    print_memory();
    
    return 0;
}