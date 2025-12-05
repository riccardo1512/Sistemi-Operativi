#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef struct node {

	int free;
	int size;
	struct node* next;
} node;

node* first;

void* my_malloc(size_t size) {

	node* cur = first;

	while(cur != NULL) {
	
	
		if(cur->free && cur->size > size + sizeof(node)) {
	
			void* cur_pos = (void*) cur;

			node* new_node = cur_pos + sizeof(node) + size;
			new_node->free = 1;
			new_node->size = cur->size - size - sizeof(node);
			new_node->next = NULL;


			cur->next = new_node;
			cur->size = size;
			cur->free = 0;


			void* output = cur_pos + sizeof(node);
			return output;
		}
		cur = cur->next;
	}
	
	return NULL;
}

void print_memory() {

	node* cur = first;

	printf("\n\n------ Inizio stampa memoria ------\n");

	for(int i = 0; cur != NULL; i++) {
	
		printf("\nnode %d", i);
		printf("\nnode->free = %d", cur->free);
		printf("\nnode->size = %d\n", cur->size);

		cur = cur->next;
	}

	printf("\n------ Fine stampa memoria ------\n\n");
}

void merge_memory() {

	node* cur = first;

	while(cur != NULL && cur->next != NULL) {
	
		if(cur->free && cur->next->free) {

			cur->size = cur->size + sizeof(node) + cur->next->size;
			cur->next = cur->next->next;
		} else {
			cur = cur->next;
		}
	}
}


void* my_free(void* ptr) {

	ptr = ptr - sizeof(node);

	node* cur = (node*) ptr;

	cur->free = 1;
}

int main(int argc, char* argv[]) {

	if(argc != 2) {

		fprintf(stderr, "Errore, sintassi: ./my_malloc <size>");
		exit(1);
	}

	size_t size = atol(argv[1]);

	
	void* start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	first = (node*) start;
	first->free = 1;
	first->size = size - sizeof(node);
	first->next = NULL;
	printf("Dimensione di un singolo node = %ld\n", sizeof(node));

	int* array1 = my_malloc(5*sizeof(int));
	printf("Dimensione array1 = %ld\n", 5*sizeof(int));
	for(int i = 0; i < 5; i++) {
	
		array1[i] = i;
	}
	
	print_memory();

	for(int i = 0 ; i < 5; i++) {

		printf("array1[%d] = %d\n", i, array1[i]);
	}


	int* array2 = my_malloc(10*sizeof(int));
	int* array3 = my_malloc(10*sizeof(int));
	int* array4 = my_malloc(10*sizeof(int));

	print_memory();

	my_free(array3);
	my_free(array4);

	print_memory();

	merge_memory();
	print_memory();
}
