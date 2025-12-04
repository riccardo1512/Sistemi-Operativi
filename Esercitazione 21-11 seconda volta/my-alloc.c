/*
VERSIONE CON while() E FUNZIONE coalescence()

	[*] sistemare il merging <- alla fine quando ho fatto my_free() mi da 2 blocchi entrambi liberi ma dovrebbero essere uniti
	[*] aggiungere al notion tutte le cose
	[*] error check my_malloc() e my_free()
	[*] my_calloc()
	[*] my_malloc(0)
	[*] my_realloc()

	[ ] manca la parte di aumentare my_heap in my_realloc()
	[ ] capire la cosa di (void *) e (char *)
	[ ] capire meglio l'algoritmo di malloc()

	[ ] esiste politica per cui sceglie nella malloc() il "buco" di memoria piu' piccolo necessario per allocare la memoria in modo
	    tale da sprecare meno memoria possibile e limitare la frammentazione
	[ ] al posto di char free potrei fare bit-packing(?) (il prof ha detto qualcosa su sta cosa per il tipo di free pero' bho)
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#define HEAP_SIZE 1024 // <- seguendo l'esempio del file my_malloc.pdf => un heap di 1024 Bytes
#define SIZE_MAX 1000 // <- 1024 - 24 [ = sizeof(Block)], tale parametro e' anche definito in <stdint.h>

/*
la struct Block contiene i metadati dei blocchi di memoria all'interno del mio heap creato tramite mmap()
e' utilizzato come header all'inizio di ogni blocco di memoria all'interno della memoria mmap-ata
infatti dopo ogni blocco sara' presente la memoria libera per l'allocazione da parte dell'utente
*/
typedef struct Block {
	size_t size;
	//int free; // flag
	unsigned char free; //<- cosi' occupo meno memoria
	struct Block *next;
} Block;

static void *my_heap = NULL; 		// ho bisogno che l'heap [la memoria che alloco con mmap()] sia una variabile globale
					// in modo tale che sia accessibile da tutte le funzione all'interno del programma

static Block *block_list = NULL;	// block_list e' una lista di Block che mi serve globale per lo stesso motivo di my_heap
					// puntera' sempre al primo header (Block) dell'heap ed e' una lista dei vari Block nell'heap
					// dove il primo elemento della lista coincide sempre col primo blocco dell'heap
					// FONDAMENTALE PER my_malloc() e my_free()

void print_heap(void) {
	printf("\n+++++++ HEAP STATE ++++++\n");

	Block *curr = block_list;
	int i = 0;

	while(curr != NULL) {
		printf("Block %d:\n", i++);
		printf("* header @ %p\n", (void *) curr);
		printf("* data   @ %p\n", (void *) ((void *) curr + sizeof(Block)));
		printf("* size   = %zu\n", curr->size); // da man size_t per utilizzarlo con printf() si usa %zu o %zx
		printf("* free   = %s\n", curr->free ? "YES" : "NO");
		printf("* next   @ %p\n", (void *) curr->next);
		if(curr->next != NULL)
			printf("-------------------------\n");
		curr = curr->next;
	}

	printf("+++++++++++++++++++++++++\n");
}

int init_memory_pool() {
	/*
	man mmap():

	SYNOPSIS
		#include <sys/mman.h>

		void *mmap(void addr[.length], size_t length, int prot, int flags, int fd, off_t offset);
		int munmap(void addr[.length], size_t length);

	RETURN VALUE
		On success, mmap() returns a pointer to the mapped area.
		On error, the value MAP_FAILED (that is, (void *) -1) is returned, and errno is set to indicate the error.

		On success, munmap() returns 0.
		On failure, it returns -1, and errno is set to indicate the error (probably to EINVAL).

	DESCRIPTION
		mmap()  creates  a new mapping in the virtual address space of the calling process.
		The starting address for the new mapping is specified in addr.
		The length argument specifies the length of the mapping (which must be greater than 0).

		If addr is NULL, then the kernel chooses the (page-aligned) address at which to create the mapping;
		this is the most portable method of creating a new mapping.

	gli argomenti di mmap() che bisogna utilizzare saranno:

	* void addr[.length] = NULL		<- lascio al kernel la decisione di dove mappare la memoria
	* size_t length = length		<- il parametro che passo a my_malloc()
	* int prot = PROT_WRITE | PROT_READ	<- la memoria deve essere acceduta per scrittura e lettura
	* int flags = MAP_ANON | MAP_PRIVATE	<- non uso file, non voglio che la memoria sia acceduta da altri processi e che sia mmap-ata su file
	* int fd = -1				<- non uso file quindi -1
	* off_t offset = 0			<- non ho offset da dargli
	*/

	my_heap = mmap(NULL, HEAP_SIZE, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
	// my_heap e' l'area di memoria che alloco inizialmente per creare una memoria virtuale
	// dove l'utente puo' allocare memoria grazie a my_malloc()

	if(my_heap == MAP_FAILED) {
		my_heap = NULL;
		block_list = NULL;
		return -1;
	}

	// inizializzazione del primo header che include l'intero blocco mappato prima (my_heap)
	block_list = (Block *) my_heap;
	block_list->size = HEAP_SIZE - sizeof(Block); // gli sottraggo la dimensione dell'header (non e' utilizzabile)
	block_list->free = 1;
	block_list->next = NULL;

	printf("\nspazio degli indirizzi virtuali mmap-ato\n");
	print_heap();
}


void coalescence(Block *freed) {

	//++++ MERGING ++++
	// controllo il caso in cui posso unire due blocchi di memoria libera per non sprecare spazio di memoria

	/*
	il meccanismo unisce 2 blocchi di memoria contigui liberi (free == 1)
	per farlo non "guarda" soltanto il blocco appena liberato e di fronte a questo ma tramite un semplice
	ciclo while() scorre tutta la block_list per cercare ogni volta 2 blocchi di memoria contigui liberi
	[probabilmente non e' il metodo piu' efficiente]
	*/
	Block *curr = block_list;
	while(curr != NULL && curr->next != NULL) {
		if(curr->free == 1 && curr->next->free == 1) {
			curr->size += curr->next->size + sizeof(Block);
			curr->next = curr->next->next;
		}
		else
			curr = curr->next;
	}
}

/*
man 3 free():

DESCRIPTION
	The free() function frees the memory space pointed to by ptr, which must have been returned by
	a previous call to malloc() or related functions.
	Otherwise, or if ptr has already been freed, undefined behavior occurs.
	If ptr is NULL, no operation is performed.

RETURN VALUE
	 The free() function returns no value, and preserves errno.
*/
void my_free(void* start_address) {
// free() ha come argomento 'void *_Nullable ptr' quindi gli passo il puntatore che punta alla memoria allocata da my_malloc() allocata prima

	if(start_address == NULL) return;

	/*
	* risale all'header del blocco sottraendo sizeof(Block)
	* segnare free = 1
	* liberare quella porzione di memoria
	* fondere eventuale blocchi liberi adiacenti (merge)
	*/

	Block *freed = (Block *)((void *)start_address - sizeof(Block)); // torno indietro all'inizio dell'header per modificarlo
	freed->free = 1; // cambio la flag ponendo il block libero (la memoria allocata in precedenza verra' sovrascritta da nuovi dati)

	coalescence(freed);
	print_heap();
}

/*
man malloc():

DESCRIPTION
	The  malloc() function allocates size bytes and returns a pointer to the allocated memory.
	The memory is not initialized.

	If size is 0, then malloc() returns a unique pointer value that can later be successfully passed to free().

RETURN VALUE
	The  malloc(), calloc(), realloc(), and reallocarray() functions return a pointer to the allocated memory,
	which is suitably aligned for any type that fits into the requested size or less.
	On error, these functions return NULL and set errno.

NOTES
	If  your  program uses a private memory allocator, it should do so by replacing malloc(), free(), calloc(), and realloc().
	The replacement functions must implement the documented glibc behaviors, including errno handling, size-zero allocations,
	and overflow checking;
	otherwise, other library routines may crash or operate incorrectly.  For example, if the replacement free()  does  not  preserve  errno,
	then  seemingly unrelated library routines may fail without having a valid reason in errno.
	Private memory allocators may also need to replace other glibc functions; see "Replacing malloc" in the glibc manual for details.

	=> NEL PROGRAMMA MANCANO LE FUNZIONI calloc() E realloc() E MANCA LA GESTIONI DEGLI ERRORI <=
*/
void *my_malloc(size_t length) {
// malloc() ha come argomenti solo 'size_t size' quindi l'unica cosa importante da dare a my_malloc() sarebbe length, come da consegna

	/*
	* scorre la lista block_list
	* allochera' memoria all'interno di un blocco libero
	* se e' troppo grande splittarlo in 2 blocchi uno allocato e un'altro lasciato libero
	* se e' quasi grande quanto tutto il mio heap allocare un unico grande blocco
	* restituisce un puntatore ai dati e non all'header (alla struct Block)
	*/

	if (block_list == NULL) {
		printf("\nblock_list == NULL => sto alla prima chiamata di my_malloc() e non ho mai ancora chiamati init_memory_pool()\n");
		if(init_memory_pool() == -1) {
			return NULL;
		}
	}

	// caso particolare in cui my_malloc(0) e restituisco un puntatore che e' lecito per my_free()
	// => creo quindi un dummy block di size = 0 dopo il primo block adatto, se non c'e' spazio nemmeno per il dummy => error e NULL
	if(length == 0) {
		Block *dummy = block_list;
		while(dummy != NULL) {
			if(dummy->free == 1 && dummy->size > sizeof(Block) + 1) {
				// ho trovato un blocco adatto => creo un nuovo blocco e lo inizializzo
				Block *after_dummy = (Block *)((void *)dummy + sizeof(Block));

				after_dummy->size = dummy->size - sizeof(Block);
				after_dummy->next = dummy->next;
				after_dummy->free = 1;

				// modifico il block che avevo trovato prima per renderlo dummy e lo restituisco
				dummy->next = after_dummy;
				dummy->free = 0;
				dummy->size = 0;

				print_heap();
				return (void *)dummy + sizeof(Block);
			}
			dummy = dummy->next;
		}
		errno = ENOMEM;
		return NULL;
	}

	// meccanismo per scorrere nella lista e trovare un blocco libero
	Block *curr = block_list;
	while(curr != NULL) {
		if(curr->free == 1 && curr->size >= length) {
		// se il blocco e' libero e puo' ospitare la memoria da allocare => sono sicuro che la memoria richiesta verra' allocata

			// **** SI SPLIT ****
			if(curr->size >= length + sizeof(Block) + 1) {
			// caso in cui la memoria si puo' splittare dove '1' sarebbero i dati utente minimi possibili da allocare dopo l'header
				// creo un nuovo blocco che sara' quello libero e verra' dopo quello occupato
				Block *nuovo = (Block *)((void *)curr + sizeof(Block) + length);
				// utilizzo (char *)/(void *) per eseguire aritmetica di puntatori e
				// calcolare precisamente l'inizio del nuovo blocco
				// => il professore preferisce il void al char perche' il void e' il tipo universale che va bene per tutto sempre
				//    mentre il tipo char anche se oggi puo' esse meglio in futuro potrebbe cambiare
				nuovo->size = curr->size - length - sizeof(Block);
				nuovo->free = 1;
				nuovo->next = curr->next; // se sto inserendo nuovo tra due block devo trasferire questa informazione e collegarli

				curr->size = length; // la memoria allocata
				curr->free = 0; // non e' piu' libera
				curr->next = nuovo; // devo collegare i block

			// **** NO SPLIT ****
			} else {
			// caso in cui la memoria non si puo' splittare visto che non ci sarebbe spazio per un nuovo blocco
				curr->free = 0;
				// uso l'intera memoria che ho a disposizione rimasta
				// => l'unica cosa che faccio e' modificare il campo free
			}
			print_heap();
			return (void *)curr + sizeof(Block); // puntatore a indirizzo spazio dati
		}
		// passo al prossimo block
		curr = curr->next;
	}
	// se si e' uscito dal while() significa che non si e' trovato spazio per allocare la memoria richiesta
	errno = ENOMEM;
	printf("non c'era spazio :(\n");
	return NULL;
}

/*
man calloc()

SYNOPSIS

	void *calloc(size_t nmemb, size_t size);

DESCRIPTION

	The calloc() function allocates memory for an array of nmemb elements of size bytes each and
	returns a pointer to the allocated  memory.
	The  memory is set to zero.

	If nmemb or size is 0, then calloc() returns a unique pointer value that can later be successfully passed to
	free().

	If the multiplication of nmemb and size would result in integer overflow, then calloc() returns an error.

RETURN VALUE
	The malloc(), calloc(), realloc(), and reallocarray() functions return a pointer to the allocated memory,
	which is suitably  aligned  for any  type  that  fits into the requested size or less.

	On error, these functions return NULL and set errno.
	Attempting to allocate more than PTRDIFF_MAX bytes is considered an error,
	as an object that large could cause later pointer subtraction to overflow.

ERRORS
       calloc(), malloc(), realloc(), and reallocarray() can fail with the following error:

       ENOMEM Out  of memory.
*/
void *my_calloc(size_t num, size_t length) {

	/*
	* moltiplica num e length per calcolare la dimensione necessaria da allocare
	* scorre la lista dei block
	* ne trova uno disponibile e adatto
	* alloca un array dinamico nello spazio trovato
	* inizializza gli elementi a 0
	* restituisce un puntatore all'array dinamico
	*/

	// caso in cui uno dei 2 input sia zero => my_calloc() restituisce un puntatore speciale "liberabile" da my_free()
	// come my_malloc(0) => restituisco la chiamata a my_malloc(0)
	if(num == 0 || length == 0) {
		return my_malloc(0);
	}

	// calcolo della vera dimensione necessaria e controllo di overflow
	if(num * length > SIZE_MAX) {
		errno = ENOMEM;
		return NULL;
	}

	size_t dim = num * length; // dimensione da allocare

	void *d_array = my_malloc(dim); // uso la my_malloc() per allocare lo spazio

	if(d_array == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	unsigned char *p = d_array; // tipo che usa 1 solo byte e nessun bit per il segno <- lo uso per spostarmi di byte in byte

	for(size_t i = 0; i < dim; i++) {
		p[i] = 0;		// azzero OGNI byte che ho allocato cosi' che quando verranno utilizzati,
					// qualsiasi sia il tipo scelto memoria diventa un array di tutti zeri
					// se avessi dichiarato p come (void *) mi avrebbe dato questo errore:
					// "warning: dereferencing 'void *' pointer" <- non puo' assegnare un valore a un void
	}

	// salto qui il print_heap(); poiche' viene gia' chiamato dalla chiamata a my_malloc() precedente
	return d_array;
}

/*
man realloc()

SYNOPSIS

       void *realloc(void *_Nullable ptr, size_t size);

DESCRIPTION

	The realloc() function changes the size of the memory block pointed to by ptr to size bytes.
	The contents of the memory will be unchanged in the range from the start of the region up to the minimum of the old and new sizes.
	If the new size is larger than the old size, the added memory will not be initialized.

	If ptr is NULL, then the call is equivalent to malloc(size), for all values of size.

	If size is equal to zero, and ptr is not NULL, then the call is equivalent to free(ptr) (but see "Nonportable behavior"
 	for  portability issues).

	Unless  ptr  is  NULL, it must have been returned by an earlier call to malloc or related functions.
	If the area pointed to was moved, a free(ptr) is done.

RETURN VALUE

	The malloc(), calloc(), realloc(), and reallocarray() functions return a pointer to the allocated memory,
	which is suitably aligned for any type that fits into the requested size or less.

	On error, these functions return NULL and set errno.
	Attempting to allocate more than PTRDIFF_MAX bytes is considered an error,
	as an object that large could cause later pointer subtraction to overflow.

	The realloc() and reallocarray() functions return NULL if ptr is not NULL and the requested size is zero;
	this is not considered an error.  (See "Nonportable behavior" for portability issues.)
	Otherwise, the returned pointer may be the same as ptr if the allocation was not moved
	(e.g., there was room to expand the allocation in-place), or different from ptr if the allocation was moved to a new address.

	If these functions fail, the original block is left untouched; it is not freed or moved.

ERRORS

	calloc(), malloc(), realloc(), and reallocarray() can fail with the following error: ENOMEM Out of memory.
*/

void *my_realloc(void *start_address, size_t length) {

	/*
	* casi speciali:
		~ start_address == NULL => my_malloc(length)
		~ length == 0 => my_free(start_address)

	* caso SHRINK => [length <= curr->size] => diminuire la dimensione:
						   se possibile splitto in due block senno' piccola riduzione e non cambio nulla

	* caso GROW   => [length > curr->size] => aumentare la dimensione

		(1) GROW IN PLACE ~ [curr->next != NULL && curr->next->free == 1 && curr->size + sizeof(Block) + curr->next->size >= length]
		    caso in cui posso aumentare la dimensione della memoria allocabile di curr senza doverlo spostare

			(1.1) NO SPLIT <- curr->next verra' inglobato da curr visto che non c'e' spazio per la creazione di un nuovo block

					[curr->size + sizeof(Block) + curr->next->size < length + sizeof(Block) + 1]

			(1.2) SPLIT <- curr->next verra' "spostato" (in verita' verra' creato un nuovo block spostato che lo sostituira')

					[curr->size + sizeof(Block) + curr->next->size >= length + sizeof(Block) + 1]

					=> si creera' Block *nuovo che sara' posizionato al byte [(void *)start_address + length]

		(2) GROW NOT IN PLACE:
		    caso in cui non ho spazio per allocare length 'in place' <- puo' succede in tre occasioni:
			(a) curr->next == NULL
			(b) curr->next->free == 0
			(c) length > curr->size + curr->next->size + sizeof(Block)

		    => my_realloc() chiama my_malloc(length) per provare ad allocare la memoria necessaria da qualche altra parte della memoria

			(2.1) my_malloc(length) fallisce => errno = ENOMEM; return NULL
			(2.2) my_malloc(length ha successo => vengono copiati dati di curr all'interno della nuova area allocata
			      e la funzione restituisce il puntatore alla nuova area di memoria e libera il block precedente (curr)
	*/

	if(start_address == NULL) { return my_malloc(length); }
	// stessa cosa di dire "alloca spazio degli indirizzi di tale misura dove vuoi" = malloc()

	if(length == 0 && start_address != NULL) { // l'utente sta azzerando la "misura" di tale spazio allocato = free()
		my_free(start_address);
		return NULL;
	}
	Block *curr = (Block *)((void *)start_address - sizeof(Block));

	// SHRINK
	if(length <= curr->size) {
		if(curr->size - length >= sizeof(Block) + 1) { // controllo se c'e' spazio per splittare [simile a my_free()]
			Block *b_next = (Block *)((void *)curr + sizeof(Block) + length);

			b_next->size = curr->size - length - sizeof(Block);
			b_next->next = curr->next;
			b_next->free = 1;

			curr->size = length;
			curr->next = b_next;

			coalescence(b_next);
		}
		/* da glibc wiki:
			If the size of the allocation is being reduced by enough to be "worth it", the chunk is split into two chunks.
			The first half (which has the old address) is returned, and the second half is returned to the arena as a free chunk.
			Slight reductions are treated as "the same size".

		=> se non si puo' splittare si ha una piccola riduzione e non va cambiato nulla restituendo il puntatore iniziale
		*/
		print_heap();
		return start_address;
	}

	// GROW
	else {
		// GROW IN PLACE
		if(curr->next != NULL && curr->next->free == 1 && curr->size + curr->next->size + sizeof(Block) >= length) {
			Block *b_next = curr->next;
			size_t tot = curr->size + b_next->size + sizeof(Block);
			// SPLIT
			if(tot >= length + sizeof(Block) + 1) {
				Block *nuovo = (Block *)((void *)start_address + length);

				nuovo->free = 1;
				nuovo->next = b_next->next;
				nuovo->size = tot - length - sizeof(Block);

				curr->next = nuovo;
				curr->size = length;
			}
			// NO SPLIT
			else {
				curr->size = tot;
				curr->next = curr->next->next;
			}

			print_heap();
			return start_address;
		}
		// GROW NOT IN PLACE
		else {
			void *new_ptr = my_malloc(length);

			if(new_ptr == NULL) {
				errno = ENOMEM;
				return NULL;
			}

			/*
			SYNOPSIS
				include <string.h>

				void *memcpy(void dest[restrict .n], const void src[restrict .n], size_t n);

			DESCRIPTION
				The memcpy() function copies n bytes from memory area src to memory area dest.
				The memory areas must not overlap.  Use memmove(3) if the memory areas do overlap.

			RETURN VALUE
				The memcpy() function returns a pointer to dest.
			*/

			memcpy(new_ptr, start_address, curr->size);

			my_free(start_address);
			print_heap();
			return new_ptr;
		}
	}
}

int main(int argc, char **argv) {

	// non serve chiamarlo qua (credo) pero' per una stampa corretta per mostrare il funzionamento meglio metterlo qua
	if(init_memory_pool() == -1) {
		perror("init_memory_pool() [mmap] failed\n");
		return 1;
	}

	printf("\nvoid *p1 = my_alloc(1000)\n");
	void *p1 = my_malloc(1000);

	printf("\nmy_free(p1)\n");
	my_free(p1);

	printf("\nvoid *p2 = my_malloc(100)\n");
	void *p2 = my_malloc(100);

	printf("\nvoid *p3 = my_malloc(200)\n");
	void *p3 = my_malloc(200);

	printf("\nmy_free(p2)\n");
	my_free(p2);

	printf("\nmy_free(p3)\n");
	my_free(p3);

	printf("\nvoid *dummy = my_malloc(0)\n");
	void *p4 = my_malloc(0);

	printf("\nmy_free(dummy)\n");
	my_free(p4);

	printf("\nint *array = (int *) my_calloc(4, sizeof(int))\n");
	int *array = (int *) my_calloc(4, sizeof(int));
	printf("\necco l'array inizializzato a 0:\n");
	for(int i = 0; i < 4; i++) {
		if(i < 3)
			printf("%d - ", array[i]);
		else
			printf("%d\n", array[i]);
	}

	printf("\nmy_realloc(array, 8 * sizeof(int)\n");
	my_realloc(array, 8 * sizeof(int));

	/*
	man munmap:

	SYNOPSIS
		int munmap(void addr[.length], size_t length);

	RETURN VALUE
		On success, munmap() returns 0.
		On failure, it returns -1, and errno is set to indicate the error (probably to EINVAL).

	=> gli argomenti di munmap() che bisogna utilizzare saranno:
		* void addr[.length] = my_heap
		* size_t length = HEAP_SIZE
	*/
	if(munmap(my_heap, HEAP_SIZE) == -1) {
		perror("munmap failed\n");
		return 1;
	}

	return 0;
}
