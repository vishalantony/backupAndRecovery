#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "includes/consts_prototypes.h"
#include "includes/filedata.h"

#define ALLOCATE_FILE_NODE() (file_node *)malloc(sizeof(file_node))
#define ALLOCATE_VERSION_NODE() (version_node *) malloc(sizeof(version_node))

file_node * fp[MAXFILES];
const int BACKUP_ERROR = -1;


/***************************************************************************/

// calculated hash value of the provided string
unsigned int hash(const char * fname) {
	unsigned int x = 0;
	while(*fname) {
		x = x*10 + *fname;
		x %= MAXFILES;
		fname++;
	}
	return x;
}

/***************************************************************************/
// call this function if filename doesn't exist.

int backup_file_not_exists(const char * filename, int v,const char * content, int fd) {
	const int BACKUP_ERROR = -1;
	int chunks_alloc = 0;
	// filename doesn't exist
	// perform all allocations and initializations
	file_node *temp = ALLOCATE_FILE_NODE();
	temp->next = fp[fd];		// add it to the beginning.
	fp[fd] = temp; 
		
	strcpy((temp->file_obj).name, filename);
	((temp->file_obj).vp) = ALLOCATE_VERSION_NODE();
	version_node * vnptr = ((temp->file_obj).vp);
	vnptr->next = NULL;
	(vnptr->v).v_no = v;
	(vnptr->v).off = 0;
	(vnptr->v).del_mark = 0;	
	
		
	int l = strlen(content);
	// enough space?
	if(l > free_chunks() * CHUNKSIZE) {
		fprintf(stderr, "ERROR: Not enough memory to backup.\n");
		return BACKUP_ERROR;
	}
	
	int i = 0, j = 0;
	while(i < l && (vnptr->v).off < MAX_NUM_CHUNKS) {
		(vnptr->v).contents[j] = (chunk *)getblock();
		chunks_alloc++;
		if((vnptr->v).contents[j] == NULL) { // getblock returned null
			(vnptr->v).off = j;
			fprintf(stderr, "ERROR: No free chunks.\n");
			return BACKUP_ERROR;
		}
		bcopy(content+i, (vnptr->v).contents[j], CHUNKSIZE);
		i += CHUNKSIZE; // copied a chunk. go to next chunk. number of characters copied is the chunksize
		j++;
	}
	(vnptr->v).off = j;
	// successfully backup up.
	return chunks_alloc;
}


/***************************************************************************/
// to write contents to an existing version 

int overwrite_version(version_node * vp, const char * content) {
	(vp->v).del_mark = 0;		// since we're overwriting, its not deleted anymore.
	int i = 0, j = 0;
	int l = strlen(content);
	int chunks_alloc = 0;
	
	if((l - (vp->v).off * CHUNKSIZE) > (int)(free_chunks() * CHUNKSIZE)) {
		fprintf(stderr, "ERROR: Not enough memory to backup.\n");
		return BACKUP_ERROR;
	}
	
	while(i < (vp->v).off && j < l) {
		strncpy((char*)(vp->v).contents[i], content+j, CHUNKSIZE);
		i++; j += CHUNKSIZE;
	}
	// either still content remaining to be written or all the content written.
	// if still more content to be written:
	while(j < l) {
		(vp->v).contents[(vp->v).off++] = (chunk *) getblock();
		if((vp->v).contents[(vp->v).off-1] == NULL) {
			fprintf(stderr, "ERROR: No free chunks.\n");
			return BACKUP_ERROR;
		}
		chunks_alloc++;
		strncpy((char*)(vp->v).contents[(vp->v).off-1], content+j, CHUNKSIZE);
		j += CHUNKSIZE;
	}
	return chunks_alloc;
}

/***************************************************************************/

// To add new version
int add_new_version(file * fileobj, version_node * vq1, version_node * vq2, int v, const char * content) {
	// check for space
	int l = strlen(content);
	int chunks_alloc = 0;
	if(l > free_chunks() * CHUNKSIZE) {
		fprintf(stderr, "ERROR: Not enough memory to backup.\n");
		return BACKUP_ERROR;
	}

	// initialize the version node
	version_node * temp_version = ALLOCATE_VERSION_NODE();
	(temp_version->v).v_no = v;
	(temp_version->v).del_mark = 0;
	(temp_version->v).off = 0;
	int i;
	for(i = 0; i < l; i++) {
		(temp_version->v).contents[(temp_version->v).off++] = (chunk *)getblock();
		if((temp_version->v).contents[(temp_version->v).off-1] == NULL) {
			fprintf(stderr, "ERROR: No free chunks.\n");
			return BACKUP_ERROR;
		}
		chunks_alloc++;
		strncpy((char*)(temp_version->v).contents[(temp_version->v).off-1], content+i, CHUNKSIZE);
		i += CHUNKSIZE;
	}
	// initialization of version node done.

	// find the place to insert the version.
	// there are 3 cases:
	// case 1: largest version number
	// case 2: somewhere in between
	// case 3: lowest version number
	
	// CASE 1:
	if(v > (vq2->v).v_no) {
		// insert in the front.
		temp_version->next = fileobj->vp;
		fileobj->vp = temp_version;
	}
	// CASE 3:
	else if(v < (vq1->v).v_no) {
		temp_version->next = vq1->next;
		vq1->next = temp_version;
	}
	// CASE 2:
	else {
		temp_version->next = vq2->next;
		vq2->next = temp_version;
	}
	return chunks_alloc;
}

/***************************************************************************/

// this function will overwrite the contents if already the file+version is present.
// returns error codes:
// -1 : error backing up.
// if successful, returns the number of chunks allocated.

int backup(const char * filename, int v, const char * content) {
	int fd = hash(filename);
	if(fp[fd] == NULL) {
		return backup_file_not_exists(filename, v, content, fd);
	}

	else {
		// 2 cases :
		// 1. already inserted the filename
		// 2. not yet inserted the filename

		// scan until we find the file (because more than 1 filenames can hash to same value)
		file_node * temp = fp[fd];
		while(temp->next != NULL && strcmp((temp->file_obj).name, filename) != 0)
			temp = temp->next;

		// CASE 1:
		if(strcmp((temp->file_obj).name, filename) == 0) {
			// check if the version exists.
			// if the version exists, overwrite.
			// if the version doesn't exist, create the version and add it to the right place.

			// we expect the version queue to be sorted in decreasing order. WHY?
			// because newer versions need to be recovered faster. If they're near the
			// head of linked list, they'll be reached faster.


			// checking if version exists:
			version_node * version_queue = (temp->file_obj).vp;
			version_node * version_queue2 = version_queue;
			while((version_queue->v).v_no > v && version_queue->next != NULL) {
				version_queue2 = version_queue;
				version_queue = version_queue->next;
			}
			if((version_queue->v).v_no == v) {
				// version exists.
				// overwrite the contents of the version
				//overwrite_version(version_node * vp, const char * content);
				return overwrite_version(version_queue, content);
			}
			else {
				// version doesn't exist.
				// the second and third argument are to remove recomputation.
				return add_new_version(&(temp->file_obj), version_queue, version_queue2, v, content);
			}
		}

		//CASE 2:
		else {		// filename doesn't exist yet.
			// backup this file as a new entry.
			return backup_file_not_exists(filename, v, content, fd);
			// successfully backed up.
		}
	}
}

void bcopy(const void * src, void * dest, size_t n) {
	memcpy(dest, src, n);
}

