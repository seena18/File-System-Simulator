#ifndef _libDisk_h
#define _libDisk_h

/* The libDisk API */

extern int openDisk(const char *filename, int nBytes);
extern int closeDisk(int disk);
extern int readBlock(int disk, int bNum, void *block);
extern int writeBlock(int disk, int bNum, const void *block);

#endif /* _libDisk_h */
