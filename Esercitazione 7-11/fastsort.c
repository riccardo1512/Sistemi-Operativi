#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/stat.h>

void usage(char *prog)
{
	fprintf(stderr, "usage: %s <input_file> <output_file>\n", prog);
	exit(1);
}

int compare(const void *a, const void *b)
{
	const rec_t *ra = (const rec_t *) a;
	const rec_t *rb = (const rec_t *) b;
	if (ra->key < rb->key)
		return -1;
	else if (ra->key > rb->key)
		return 1;
	else
		return 0;
}

int main(int argc, char *argv[])
{

	char *input_path = argv[1];
	char *output_path = argv[2];

	// Apertura del file di input
	int ifd = open(input_path, O_RDONLY);
	if (ifd == -1)
	{
		fprintf(stderr, "Errore apertura file\n"); // uso la fprintf per specificare di stampare su standard error
		exit(1);								   // codice di errore, ci dovrebbe essere una tabella
	}

	// mi calcolo le statistiche
	struct stat *statbuf = malloc(sizeof(struct stat));
	fstat(ifd, statbuf);
	unsigned int size = statbuf->st_size;

	int num_record = size / sizeof(rec_t);
	printf("numero record = %zu", num_record);

	// creo nell'heap dove salvarmi tutto
	rec_t *buffer = malloc(num_record * sizeof(rec_t));		// per variabili grandi usare l'heap, prk lo stack è piccolo
	rec_t record;

	/* leggo i record ad uno ad uno finchè la read legge */
	for(int i = 0; i < num_record; i++) {
		
		read(ifd, &record, sizeof(rec_t));
		buffer[i] = record;
	}
	// altra chiamata mmap, crea array che mappa ogni singolo byte del file, è una figata pazzesca, serve sufficiente memoria per mappare tutto il file

	// sorto usando la funzione compare
	qsort(buffer, num_record, sizeof(rec_t), compare);

	// scrivo in output
	int fd_out = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	write(fd_out, buffer, size);

	close(fd_out);

	(void)close(ifd);

	return 0;
}
