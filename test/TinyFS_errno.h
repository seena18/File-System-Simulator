#ifndef _TinyFS_errno_h
#define _TinyFS_errno_h

/* Define TinyFS error codes here */

/* Incorrect block size */
#define EBLKSZ -1
/* Disk does not exist */
#define ENODSK -2
/* Disk is not opened */
#define EDSKOP -3
/* Out of disk bounds */
#define EDSKBD -4
/* Memory allocation failure (malloc,realloc) */
#define EALLOC -5
/* File Error */
#define ENOFIL -6
/* Out of file bounds */
#define EFILBD -7
/* File name too large */
#define EFILNM -8
/* File size too large */
#define EFILLG -9
/* File is not opened */
#define EFILOP -10
/* File EOF */
#define EEOF -11
/* File system already mounted */
#define EFSMT -12
/* File system invalid */
// e.g. Magic number invalid
#define EFSIV -13
/* File not mounted*/
#define EFSNMT -14




#endif /* _TinyFS_errno_h */
