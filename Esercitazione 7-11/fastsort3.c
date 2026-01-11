#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sort.h"

// 29 min 20 sec

int compare(const void* p1, const void* p2) {

    rec_t* r1 = (rec_t*) p1;
    rec_t* r2 = (rec_t*) p2;

    return (r1->key - r2->key);
}

int main(int argc, char* argv[]) {

    if(argc != 3) {

        fprintf(stderr, "Errore, sintassi: ./fastsort <file_di_input> <file_di_output>\n");
        exit(1);
    }

    char* file_di_input = argv[1];
    char* file_di_output = argv[2];


    struct stat statistiche;
    stat(file_di_input, &statistiche);


    int fd_in = open(file_di_input, O_RDONLY);

    int numero_record = statistiche.st_size / sizeof(rec_t);

    rec_t* buffer = malloc(statistiche.st_size);
    rec_t record;

    for(int i = 0; i < numero_record; i++) {

        read(fd_in, &record, sizeof(rec_t));
        buffer[i] = record;
    }
    

    qsort(buffer, numero_record, sizeof(rec_t), compare);


    int fd_out = open(file_di_output, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    for(int i = 0; i < numero_record; i++) {

        record = buffer[i];
        write(fd_out, &record, sizeof(rec_t));
    }
    


    return 0;
}