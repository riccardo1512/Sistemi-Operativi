#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "queque.h"

#define QUEUE_SIZE = 1000

int durata = 0;     // Durata della simulazione in termini di job da creare
int creati = 0;     // Quanti job ho creato
int counter = 0;     // Quanti job ho in coda


void* produttore(void* arg);
void* consumatore(void* arg);

void* produttore(void* arg) {

    Queue* q = (Queue*) arg;

    while(1) {

        if(creati >= durata) {
            break;
        }

        while(counter >= QUEUE_SIZE) {

            sched_yield();
        }

        Process* p = malloc(sizeof(Process));
        p->id = creati;
        creati++;

        p->exec_time = random() % (int) 1e6;

    }
    return NULL;
}

int main(int argc, char* argv[]) {

    // argomenti = numero processi (quanti job creati da consumatori), numero cpu

    durata = atoi(argv[1]);
    int ncpu = atoi(argv[2]);

    Queue* q = malloc(sizeof(Queue));
    initializeQueue(q);


    pthread_t produttore;
    pthread_t consumatore[ncpu];

    pthread_create(&produttore, NULL, produttore, q);

    for(int i = 0; i < ncpu; i++) {

        pthread_create(&consumatore[i], NULL, consumatore, q);
    }

    pthread_join(produttore, NULL);
    for(int i = 0; i < ncpu; i++) {

        pthread_join(consumatore[i], NULL);
    }


    return 0;
}