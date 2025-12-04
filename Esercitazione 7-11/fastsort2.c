#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "sort.h"

int compare(const void* a, const void* b) {

    rec_t* rec_a = (rec_t*) a;
    rec_t* rec_b = (rec_t*) b;

    return (rec_a->key - rec_b->key);
}

int main(int argc, char* argv[]) {

    if(argc != 3) {

        fprintf(stderr, "Errore, sintassi: ./fastsort <file_di_input> <file_di_output>");
        exit(1);
    }

    char* file_di_input = argv[1];
    char* file_di_output = argv[2];

    struct stat statistiche;
    stat(file_di_input, &statistiche);
    size_t size = statistiche.st_size;
    int num_record = size / sizeof(rec_t);


    int fd_in = open(file_di_input, O_RDONLY);
    rec_t* buffer = malloc(size);
    rec_t record;
    for(int i = 0; i < num_record; i++) {
		
		read(fd_in, &record, sizeof(rec_t));
		buffer[i] = record;
	}

    qsort(buffer, num_record, sizeof(rec_t), compare);

    int fd_out = open(file_di_output, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    for(int i = 0; i < num_record; i++) {

        record = buffer[i];
        write(fd_out, &record, sizeof(rec_t));
    }
    


    close(fd_in);
    close(fd_out);


    return 0;
}