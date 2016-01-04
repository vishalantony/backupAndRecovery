#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "includes/consts_prototypes.h"
#include "includes/filedata.h"

extern file_node * fp[MAXFILES];

// the delete function.
// this function marks the file+version for deletion if it exists.
// once marked as delete, the file+version cannot be recovered 
// unless again marked as not deleted.

// returns -1 if an error occurs.
// else returns 0.

int delete(const char * filename, int ver) {
	unsigned int fd = hash(filename);
	file_node* temp = fp[fd];
	
	while(temp != NULL) {
		if(strcmp((temp->file_obj).name, filename) == 0) 
			break;
		temp = temp->next;
	}
	
	// filename itself doesn't exist.
	if(temp == NULL) {
		fprintf(stderr, "ERROR: No such file.");
		return -1;
	}
	
	// filename exists.
	// scan if version exists.
	version_node * tempv = (temp->file_obj).vp;
	while((tempv->v).v_no > ver)
		tempv = tempv->next;
	
	// version found?
	if((tempv->v).v_no == ver) {
		(tempv->v).del_mark = 1;	// deletion marked.
	}
	
	else {
		fprintf(stderr, "ERROR: No such file+version.");
		return -1;
	}
	return 0;
}


/***************************************************************************/

// the prune function.
// this function frees up the space taken by the file+version.

// returns -1 if an error occurs.
// else returns 0.

int prune(const char * filename, int ver) {
	unsigned int fd = hash(filename);
	file_node* temp = fp[fd];
	file_node* temp2 = temp;
	
	while(temp != NULL) {
		if(strcmp((temp->file_obj).name, filename) == 0) 
			break;
		temp2 = temp;
		temp = temp->next;
	}
	
	// filename itself doesn't exist.
	if(temp == NULL) {
		fprintf(stderr, "ERROR: No such file.");
		return -1;
	}
	
	// filename exists.
	// scan if version exists.
	version_node * tempv = ((temp->file_obj).vp);
	version_node * tempv2 = ((temp->file_obj).vp);
	while((tempv->v).v_no > ver) {
		tempv2 = tempv;
		tempv = tempv->next;
	}
	
	// version found?
	if(tempv == NULL || (tempv->v).v_no != ver) {
		fprintf(stderr, "ERROR : No such file+version.");		// not found.
		return -1;
	}
	
	// version found:
	// delete version. if no more versions exists, delete file also.
	
	if(tempv == ((temp->file_obj).vp)) {	// first in the list. 
		((temp->file_obj).vp) = tempv->next;
	}
	else {
		tempv2->next = tempv->next;
	}
	
	int i;
	// deallocate chunks
	for(i = 0; i < (tempv->v).off; i++) {	
		dealloc((tempv->v).contents[i]);
		(tempv->v).contents[i] = NULL;
	}
	
	// deallocate version node.
	free(tempv);
	
	// was the only version? if yes, delete file also.
	if(((temp->file_obj).vp) == NULL) {
		if(fp[fd] == temp) {	// first in the filename list
			fp[fd] = temp->next;
			free(temp);
		}
		else {
			temp2->next = temp->next;
			free(temp);
		}
	}
	
	return 0;
}

/***************************************************************************/

// called by the terminator function.
// frees up the allocated files, versions, etc.

void free_versions(version_node * vnptr) {
	version_node * temp = vnptr;
	while(vnptr != NULL) {
		temp = vnptr->next;
		printf("... freeing version %d\n", (vnptr->v).v_no);
		int i;
		for(i = 0; i < (vnptr->v).off; i++) {
			dealloc((vnptr->v).contents[i]);
			(vnptr->v).contents[i] = NULL;
		}
		free(vnptr);
		vnptr = temp;
	}
}

void free_files(void) {
	int i;
	for(i = 0; i < MAXFILES; i++) {
		if(fp[i] == NULL) continue;
		printf("freeing file %s\n", (fp[i]->file_obj).name);
		free_versions((fp[i]->file_obj).vp);
		free(fp[i]);
	}
}
