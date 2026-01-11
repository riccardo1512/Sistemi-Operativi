#include <stdio.h>
#include "queue.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

// 1h 11 min

// 10 processi/s
// tempo esecuzione casuale < 1s
// < 1000 processi in coda
// FIFO

int durata;
int ncpu;
Queue coda;
int numeroJobInCoda = 0;
int maxCoda = 1000;
int consumati = 0;
time_t turnarountCum = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void* produttore(void* par) {

    for(int i = 0; i < durata; i++) {

        pthread_mutex_lock(&lock);
        while(numeroJobInCoda >= 1000) {
            pthread_mutex_unlock(&lock);
            sched_yield();
            pthread_mutex_lock(&lock);
        }
        
        pthread_mutex_unlock(&lock);
        Process* newProcess = malloc(sizeof(Process));
        newProcess->id = i;
        newProcess->exec_time = random() % 1000000;
        gettimeofday(&(newProcess->arrival), NULL);
        pthread_mutex_lock(&lock);
        enqueue(&coda, newProcess);
        numeroJobInCoda++;
        printf("Il produttore ha incodato il Process %ld\n", newProcess->id);
        pthread_mutex_unlock(&lock);

        usleep(100000);
    }

    printf("Il produttore ha finito il suo lavoro\n");
    return NULL;
}


void* consumatore(void* par) {

    while(1) {

        pthread_mutex_lock(&lock);
        if(consumati == durata) {
            printf("Un consumatore ha finito il suo lavoro\n");
            pthread_mutex_unlock(&lock);
            break;
        }

        while(numeroJobInCoda <= 0 && consumati != durata) {
            pthread_mutex_unlock(&lock);
            sched_yield();
            pthread_mutex_lock(&lock);
        }

        if(consumati == durata) {
            printf("Un consumatore ha finito il suo lavoro\n");
            pthread_mutex_unlock(&lock);
            break;
        }

        
        Process* process = dequeue(&coda);
        consumati++;
        numeroJobInCoda--;
        pthread_mutex_unlock(&lock);
        gettimeofday(&(process->start), NULL);
        printf("Un consumatore sta eseguendo il Process %ld\n", process->id);
        usleep(process->exec_time);
        gettimeofday(&(process->end), NULL);
        printf("Un consumatore ha finito di eseguire il Process %ld\n", process->id);
        pthread_mutex_lock(&lock);
        time_t turnaround = (process->end.tv_sec*1000000 + process->end.tv_usec) - (process->arrival.tv_sec*1000000 + process->arrival.tv_usec);
        turnarountCum += turnaround;
        free(process);
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}


// durata simulazione (numero di job che arrivano)
// numero cpu
int main(int argc, char* argv[]) {

    if(argc != 3) {

        fprintf(stderr, "Errore, sintassi: ./simulatore <durata> <numCpu>\n");
        exit(1);
    }

    initializeQueue(&coda);

    durata = atoi(argv[1]);
    ncpu = atoi(argv[2]);

    pthread_t prod;
    pthread_t cons[ncpu];

    pthread_create(&prod, NULL, produttore, NULL);
    for(int i = 0; i < ncpu; i++) {

        pthread_create(&cons[i], NULL, consumatore, NULL);
    }

    pthread_join(prod, NULL);
    for(int i = 0; i < ncpu; i++) {

        pthread_join(cons[i], NULL);
    }

    printf("Il turnaround medio è: %ld microsecondi\n", turnarountCum/durata);





    return 0;
}