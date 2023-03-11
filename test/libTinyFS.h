#include <sys/types.h>

#ifndef _libTinyFS_h
#define _libTinyFS_h

/* The libTinyFS API */

extern int tfs_mkfs(const char *filename, size_t nBytes);
extern int tfs_mount(const char *filename);
extern int tfs_unmount(void);

typedef int fileDescriptor;

extern fileDescriptor tfs_openFile(const char *name);
extern int tfs_closeFile(fileDescriptor fd);
extern int tfs_writeFile(fileDescriptor fd,
    const char *buffer, size_t size);
extern int tfs_deleteFile(fileDescriptor fd);

extern int tfs_readByte(fileDescriptor fd, char *buffer);
extern int tfs_seek(fileDescriptor fd, off_t offset);

/* Various well-defined extensions */

extern int tfs_rename(fileDescriptor fd, const char *newName);

extern int tfs_createDir(const char *dirName);
extern int tfs_removeDir(const char *dirName);
extern int tfs_removeAll(const char *dirName);

extern int tfs_makeRO(const char *name);
extern int tfs_makeRW(const char *name);

#endif /* _libTinyFS_h */
