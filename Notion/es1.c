#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>


typedef struct thread_arg {

    int id;
    char* nome_file;
} thread_arg;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char public_string[100];
int public_id;


void* routine(void* arg) {

    thread_arg* argomenti = (thread_arg*) arg;

    int id = argomenti->id;
    char* nome_file = argomenti->nome_file;
    

    int fd_out = open(nome_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    while(public_id != -2) {


        if(public_id == id) {

            write(fd_out, public_string, strlen(public_string)*sizeof(char));
            pthread_mutex_lock(&lock);
            public_id = -1;
            pthread_mutex_unlock(&lock);

            printf("Il thread con id %d ha scritto la stringa %s sul file %s\n", id, public_string, nome_file);
        }

        sched_yield();
    }

    close(fd_out);

    return NULL;
}



int main(int argc, char* argv[]) {

    if(argc < 3) {

        fprintf(stderr, "Errore, sintassi: ./a.out <nomeFile1> <nomeFile2> <...>\n");
        exit(1);
    }

    int numero_file = argc - 1;
    public_id = -1;


    pthread_t* array = malloc(sizeof(pthread_t) * numero_file);
    for(int i = 0; i < numero_file; i++) {

        thread_arg* argomenti = malloc(sizeof(thread_arg));
        argomenti->id = i+1;
        argomenti->nome_file = argv[i+1];

        pthread_create(&array[i], NULL, routine, argomenti);
    }

    while(1) {

        while(public_id != -1) {

            sched_yield();
        }

        char stringa[100]; 
        scanf("%s", &stringa);

        if(strcmp(stringa, "quit") == 0) {

            public_id = -2;
            break;
        }
        else {

            pthread_mutex_lock(&lock);
            strcpy(public_string, stringa);
            public_id = random() % numero_file + 1;
            pthread_mutex_unlock(&lock);
        }
    }


    for(int i = 0; i < numero_file; i++) {

      
        pthread_join(array[i], NULL);
    }

    printf("Tutti i thread hanno finito la loro esecuzione\n");



    return 0;
}