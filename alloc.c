#include <stdio.h>
#include <stdlib.h>
#include "includes/consts_prototypes.h"
#include <string.h>

// data private to this file

static int n_allocated, n_free;
static chunk * freememqueue[MAX_NUM_CHUNKS];
static int front, back;

// end of private data.

// does the initial setup necessary.
// our own function to allocate memory.
static int f = 0;

void init_freememqueue(void) {			// to be called when before any backup or recovery.
	if(f) return;
	int i;
	for(i = 0; i < MAX_NUM_CHUNKS; i++) {
		freememqueue[i] = (chunk *) malloc(sizeof(chunk));
	}
	front = 0;
	back = 0;
	n_allocated = 0;
	n_free = MAX_NUM_CHUNKS;
	f = 1;
}

chunk * getblock(void) {
	if(n_free == 0) 
		return NULL;
	chunk *ptr = freememqueue[front];
	front++;
	front %= MOD;
	n_allocated++;
	n_free--;
	memset(ptr, 0, CHUNKSIZE);
	return ptr;
}

void dealloc(chunk * ptr) {
	if(ptr == NULL) return;
	if(n_allocated == 0) return;
	freememqueue[back] = ptr;
	back = (back+1)%MOD;
	n_free++;
	n_allocated--;
}

unsigned int free_chunks(void) {
	return n_free;
}


/***************************************************************************/

// called by terminator function.
// frees up the allocated memory before terminating the program
void free_memory(void) {
	int i;
	for(i = 0; i < MAX_NUM_CHUNKS; i++) {
		free(freememqueue[i]);
	}
}
