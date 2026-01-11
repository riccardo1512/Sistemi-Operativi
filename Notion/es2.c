#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

// Struttura per passare i parametri ai thread
typedef struct {
    char *filename;
    int start_idx; // Indice da cui partire (in numero di interi)
    int count;     // Quanti interi leggere
    int threshold;
} t_args;

void* routine(void* arg) {
    t_args *args = (t_args*)arg;
    
    // Allocazione del risultato (necessaria per ritornare il valore al main)
    int *local_count = malloc(sizeof(int));
    *local_count = 0;

    // Ogni thread apre il file per conto suo per avere un cursore indipendente
    int fd = open(args->filename, O_RDONLY);
    
    // Spostiamo il cursore al punto giusto: offset = indice * 4 byte
    lseek(fd, args->start_idx * sizeof(int), SEEK_SET);

    int val;
    for (int i = 0; i < args->count; i++) {
        read(fd, &val, sizeof(int));
        if (val > args->threshold) {
            (*local_count)++;
        }
    }

    close(fd);
    free(arg); // Liberiamo la struct degli argomenti
    pthread_exit((void*)local_count);
}

int above_threshold(char *filename, int n, int threshold) {
    struct stat st;
    stat(filename, &st); // Recuperiamo info sul file

    int total_ints = st.st_size / sizeof(int);
    
    // Calcolo divisione lavoro
    int chunk = total_ints / n;
    int resto = total_ints % n;
    
    pthread_t *threads = malloc(n * sizeof(pthread_t));
    int current_idx = 0;

    // Creazione Thread
    for (int i = 0; i < n; i++) {
        t_args *args = malloc(sizeof(t_args));
        args->filename = filename;
        args->threshold = threshold;
        
        // Assegniamo il chunk base.
        int n_to_read = chunk;
        
        // Se è l'ultimo thread, aggiungiamo tutto il resto
        if (i == n - 1) {
            n_to_read += resto;
        }
        
        args->start_idx = current_idx;
        args->count = n_to_read;
        
        current_idx += n_to_read;

        pthread_create(&threads[i], NULL, routine, args);
    }

    int total_result = 0;

    // Join e somma
    for (int i = 0; i < n; i++) {
        int *ret_val;
        // pthread_join riempie ret_val con l'indirizzo ritornato dal thread
        pthread_join(threads[i], (void**)&ret_val);
        
        total_result += *ret_val;
        free(ret_val); // Importante: liberiamo la memoria allocata nel thread
    }

    free(threads);

    printf("%d\n", total_result);
    return total_result;
}