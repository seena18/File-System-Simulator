#include <stdio.h>

#include "tinyFS.h"
#include "TinyFS_errno.h"
#include "libTinyFS.h"

/* This is where you fully exercise the tinyFS library */


int main(int argc, char *argv[])
{
	tfs_mkfs("disk1.dsk",2048);
    tfs_mount("disk1.dsk");
	fileDescriptor fd1 = tfs_openFile("one");
	fileDescriptor fd2 = tfs_openFile("two");
	fileDescriptor fd3 = tfs_openFile("three");
	tfs_writeFile(fd1,"writing to one",252);
	tfs_writeFile(fd2,"writing to two",252);
	tfs_writeFile(fd3,"writing to three",252);
	tfs_readdir();
	tfs_rename(fd3,"four"); 
	tfs_readdir();
	
    fprintf(stderr,"Creation date: %d\n",tfs_readFileInfo(fd1));
}
