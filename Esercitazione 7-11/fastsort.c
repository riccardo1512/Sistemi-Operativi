#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/stat.h>




void usage(char *prog) {
    fprintf(stderr, "usage: %s <input_file> <output_file>\n", prog);
    exit(1);
}


int compare(const void* a,const void* b) {
    const rec_t* ra = (const rec_t*) a;
    const rec_t* rb = (const rec_t*) b;
    if (ra->key < rb->key)
        return -1;
    else if (ra->key > rb->key)
        return 1;
    else
        return 0;
}

int main(int argc, char* argv[]) {

 
	
	char *inFile = argv[1];
	char *output_file = argv[2];

    // open and create output file
    int fd = open(inFile, O_RDONLY);
    if (fd < 0) {
	perror("open");
	exit(1);
    }


	
	// mi calcolo le statistiche
	struct stat statistiche;
	 fstat(fd, &statistiche);


	off_t file_size = statistiche.st_size;


	size_t numeroRecord = file_size / sizeof(rec_t);
	printf("numero record = %zu", numeroRecord);

	// creo nell'heap dove salvarmi tutto
	rec_t* records = malloc(file_size);
	read(fd, records, file_size);

	// sorto usando la funzione compare
	qsort(records, numeroRecord, sizeof(rec_t), compare);

	// scrivo in output
	int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	write(fd_out, records, file_size);

	close(fd_out);

    
    (void) close(fd);

    return 0;

}
