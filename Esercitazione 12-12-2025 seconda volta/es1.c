#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>


int* pila;
int count;
int size;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void push(int n) {

    pila[count] = n;
    count++;
}

int pop() {

    count--;
    int ret = pila[count];

    return ret;
}

void* produttore(void* arg) {

    while(1) {

        usleep(random() % 1000000);

        pthread_mutex_lock(&lock);
        while(count >= size) {
            pthread_mutex_unlock(&lock);
            sched_yield();
            pthread_mutex_lock(&lock);
        }

        // quanti = 1
        // numero elementi nella pila = 9
        // count = 9
        // [0, 1, 2, 3, 4, 5, 6, 7, 8, x]
        if(count < size) {

            int quanti = random() % (size - count) + 1;
            
            printf("Il produttore vuole inserire %d elementi\n", quanti);

            for(int i = 0; i < quanti; i++) {

                push(quanti);
                printf("Il produttore inserisce %d\n", quanti);
            }
            pthread_mutex_unlock(&lock);
        }
    }

    return NULL;
}

void* consumatore(void* arg) {

    while(1) {

        usleep(random() % 1000000);
        
        pthread_mutex_lock(&lock);
        while(count == 0) {
            pthread_mutex_unlock(&lock);
            sched_yield();
            pthread_mutex_lock(&lock);
        }

        if(count > 0) {

            int quanti = random() % count;
            printf("Il consumatore vuole leggere %d elementi\n", quanti+1);
            // quanti = 3
            // numero elementi nella pila = 3
            // count = 3
            // [0, 1, 2, x, x, x, x, x, x, x]

            for(int i = 0; i <= quanti ; i++) {

                int n = pop();
                printf("Il consumatore ha letto %d\n", n);
            }
            pthread_mutex_unlock(&lock);
        }        
    }

    return NULL;
}

int main(int argc, char* argv[]) {

    size = 10;
    pila = malloc(size*sizeof(int));
    count = 0;

    pthread_t prod;
    pthread_t cons;

    pthread_create(&prod, NULL, produttore, NULL);
    pthread_create(&cons, NULL, consumatore, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    return 0;
}