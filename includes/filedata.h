#ifndef _H_FILEDATA
#define _H_FILEDATA

#include "consts_prototypes.h"

typedef struct version {
	int v_no;		// last written place in the contents array + 1
	int del_mark;						// 0 means not deleted. 1 means deleted
	chunk * contents[MAX_NUM_CHUNKS];		// list of chunks for the file+version
	int off; // last filled chunk
} version;

typedef struct version_node {
	struct version_node * next;
	version v;
} version_node;

typedef struct file {
	char name[1024];
	version_node * vp;		// linked list of versions
} file;

typedef struct file_node {
	file file_obj;
	struct file_node * next;
} file_node;

#endif
