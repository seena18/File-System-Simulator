#ifndef _tfs_internal_h
#define _tfs_internal_h

int mounted = -1;
typedef int fileDescriptor;
int filecount=0;
unsigned char** files;
int* offset_table;
struct timeval tv;

/* Data structures and APIs that are internal to libTinyFS */

#endif /* _tfs_internal_h */
