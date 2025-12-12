#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


int* pila;
int count;
int size;

void push(int n) {

	pila[count] = n;
	count++;
}

int pop() {

	count--;
	return pila[count];
}

void* produttore(void* arg) {

	while(1) {

		usleep(random() % 1000000);

		if(count < size) { // la pila è vuota

			int number = (int) random();
			push(number);

			printf("Il produttore ha inserito il numero %d\n", number);
		}
	}

	return NULL;
}

void* consumatore(void* arg) {

	while(1) {

		usleep(random() % 1000000);

		if(count > 0) {

			int numberRead = random() % count + 1;
			printf("Il consumatore vuole leggere %d elementi\n", numberRead);

			for(int i = 0; i < numberRead; i++) {

				int n = pop();
				printf("Il consumatore ha letto il %d numero con valore %d\n", i, n);
			}
		}

	}

	return NULL;
}

int main(int argc, char* argv[]) {


	pila = malloc(10*sizeof(int));
	size = 10;
	count = 0;

	pthread_t prod, cons;

	pthread_create(&prod, NULL, produttore, "produttore");
	pthread_create(&cons, NULL, consumatore, "consumatore");

	pthread_join(prod, NULL);
	pthread_join(cons, NULL);

	return 0;
}
