#ifndef _H_BACKUP_RECOV
#define _H_BACKUP_RECOV

#define MAXFILES 1024
#define CHUNKSIZE 16
#define MAX_MEM 256
#define MAX_NUM_CHUNKS MAX_MEM/CHUNKSIZE
#define MOD MAX_NUM_CHUNKS

typedef struct chunk {
	char chunk_space[CHUNKSIZE];
} chunk;

chunk * getblock(void);
unsigned int free_chunks(void);
void init_freememqueue(void);
void free_files(void);
void free_memory(void);
unsigned int hash(const char * fname);
int recover(const char * filename, int ver, char * buff);

#endif
