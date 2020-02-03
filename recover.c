#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "includes/consts_prototypes.h"
#include "includes/filedata.h"

extern file_node * fp[MAXFILES];

// how recover works:
// if filename+version exists, copy to buf.
// else copy the version lesser than the specified which is not
// marked for delete.

// buff is expected to have enough memory.

//returns -1 on error.
// else number of chunks recovered.
int recover(const char * filename, int ver, char * buff) {
	int fd = hash(filename);
	file_node * temp = fp[fd];
	while(temp != NULL && strcmp((temp->file_obj).name, filename))
		temp = temp->next;
	if(temp == NULL) {
		fprintf(stderr, "ERROR: No such file.");
		return -1;
	}
	version_node * vnptr = (temp->file_obj).vp;
	while(vnptr != NULL && ((vnptr->v).v_no > ver || (vnptr->v).del_mark == 1)) {
		vnptr = vnptr->next;
	}
	if(vnptr == NULL) {
		fprintf(stderr, "ERROR: Can't find the version you're looking for\n");
		return -1;
	}
	// found the version.
	int i = 0, j = 0;
	for(i = 0; i < (vnptr->v).off; i++) {
		memcpy(buff+j, (vnptr->v).contents[i], CHUNKSIZE);
		j += CHUNKSIZE;
	}
	buff[j] = '\0';
	return i;
}
