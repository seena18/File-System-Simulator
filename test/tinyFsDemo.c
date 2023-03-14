#include <stdio.h>

#include "tinyFS.h"
#include "TinyFS_errno.h"
#include "libTinyFS.h"

/* This is where you fully exercise the tinyFS library */


int main(int argc, char *argv[])
{
	tfs_mkfs("disk1.dsk",1024);
    int r = tfs_mount("disk1.dsk");
	fileDescriptor fd = tfs_openFile("test");
	tfs_writeFile(fd,"WRITING TO FILE TEST",252);
    fprintf(stderr,"Mount result: %d\n",r);
}
