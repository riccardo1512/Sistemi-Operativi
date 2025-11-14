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

	// Calcolo le statistiche
	struct stat *statbuf = malloc(sizeof(struct stat));
	fstat(ifd, statbuf);
	unsigned int size = statbuf->st_size;

	int num_record = size / sizeof(rec_t);
	printf("numero record = %d", num_record);

	// Creo nell'heap dove salvarmi tutto
	rec_t *buffer = malloc(num_record * sizeof(rec_t));		// per variabili grandi usare l'heap, prk lo stack è piccolo
	rec_t record;

	/* Leggo i record ad uno ad uno finchè la read legge */
	for(int i = 0; i < num_record; i++) {
		
		read(ifd, &record, sizeof(rec_t));
		buffer[i] = record;
	}
	// altra chiamata possibile è mmap, crea array che mappa ogni singolo byte del file, è una figata pazzesca, serve sufficiente memoria per mappare tutto il file

	// Sorto usando la funzione compare
	qsort(buffer, num_record, sizeof(rec_t), compare);

	// Apertura del file di output e scrittura
	int ofd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	for(int i = 0; i < num_record; i++) {
		
		write(ofd, &buffer[i], sizeof(rec_t));
		buffer[i] = record;
	}
	write(ofd, buffer, size);

	// Chiusura
	close(ifd);
	close(ofd);

	return 0;
}
