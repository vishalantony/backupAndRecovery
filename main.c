/**
 * BACKUP AND RESTORATION OF A FILE.
 * ASKED IN COMMVAULT 2nd ROUND FOR THE YEAR 2015 (Campus Recruitment).
 * 
 * OBJECTIVES:
 * 1. Backup and restore a file.
 * 2. Add versioning. A file can have multiple versions.
 * 3. Delete : mark for delete. Don't free up the file contents.
 * 4. Pruning : free the file contents.
 * 
 * Read and write should be done in terms of chunks (say 16 bytes).
 * Try to make your solution scalable.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "includes/consts_prototypes.h"
#include "includes/filedata.h"

extern file_node * fp[MAXFILES];

void terminator(void) {
	printf("\n\n\n******** Terminating ********\n\n");
	free_files();
	free_memory();
}

int main(void) {
	init_freememqueue();
	char buf[1024]; // recover to this
	int i = backup("uvce_cse.pdf", 1, "I'm so happy for all that the God has given me!");
	printf("Used %d chunks for \"%s\" -v=%d.\n", i, "uvce_cse.pdf", 1);
	
	i = backup("uvce_cse.pdf", 3, "I'm so sad for all that the world has given me!");
	printf("Used %d chunks for \"%s\" -v=%d.\n", i, "uvce_cse.pdf", 3);
	
	i = recover("uvce_cse.pdf", 2, buf);
	printf("\nRecovering contents of \"%s\" -v=%d...\n", "uvce_cse.pdf", 2);
	printf("Recovered : %s\nRetrieved %d chunks\n", buf, i);
	
	i = delete("uvce_cse.pdf", 1);
	if(i == -1)
		printf("Couldn't mark %s -v=%d for delete!\n", "uvce_cse.pdf", 1);
	else
		printf("%s -v=%d marked for deletion.\n", "uvce_cse.pdf", 1);
	
	i = prune("uvce_cse.pdf", 1);
	if(i == -1)
		printf("Couldn't prune the contents of %s -v=%d.\n", "uvce_cse.pdf", 1);
	else
		printf("Pruned contents of %s -v=%d.\n", "uvce_cse.pdf", 1);
		
	terminator();
	return 0;
}
