#ifndef _TinyFS_errno_h
#define _TinyFS_errno_h

/* Define TinyFS error codes here */

/* Incorrect block size */
//Use for openDisk() if 0 < nbytes < 256
#define EBLKSZ -1
/* Disc does not exist */
//Use for openDisk() if nbytes == 0 but no existing disk, readBlock, writeBlock
#define ENODSK -2
/* Disk is not opened */
//Use for closeDisk(), readBlock(), writeBlock()
#define EDSKOP -3
/* Out of disk bounds */
// e.g. block number greater than disk size
#define EDSKBD -4
/* Memory allocation failure (malloc,realloc) */
#define EALLOC -5
/* File Error */
// File not found or any other file error
#define ENOFIL -6
/* Out of file bounds */
// e.g. readbyte offset > file size
#define EFILBD -7
/* File name too large */
// tfs_openFile, rename
#define EFILNM -8
/* File size too large */
// e.g. file size > free block space
#define EFILLG -9
/* File is not opened */
// tfs_closeFile(), tfs_writeFile(), tfs_readByte(), tfs_seek()
#define EFILOP -10
/* File EOF */
#define EEOF -11
/* File system already mounted */
// tfs_mount()
#define EFSMT -12
/* File system invalid */
// e.g. Magic number invalid
#define EFSIV -13
/* File not mounted*/
// calling tfs_unmount() when nothing mounted
#define EFSNMT -14




#endif /* _TinyFS_errno_h */
