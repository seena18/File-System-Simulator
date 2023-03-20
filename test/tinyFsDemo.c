#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinyFS.h"
#include "TinyFS_errno.h"
#include "libTinyFS.h"

/* This is where you fully exercise the tinyFS library */


int main(int argc, char *argv[])
{
	// Attempt to unmount a file system that hasn't been mounted yet
	fprintf(stderr, "An attempt to unmount a file system that isn't mounted...\n");
	tfs_unmount();

	// Mounts the file system. If the disk does not exist, create one and mount it.
    if(tfs_mount(DEFAULT_DISK_NAME) < 0){
		tfs_mkfs(DEFAULT_DISK_NAME,DEFAULT_DISK_SIZE);
		int val;
		if((val = tfs_mount(DEFAULT_DISK_NAME)) < 0) {
			fprintf(stderr, "Failed to open disk\n");
			fprintf(stderr, "error val: %d", val);
			return val;
		}
	}

	fileDescriptor fd1 = -1;

	// An attempt to read from a closed file
	
	fprintf(stderr, "\nAttempt #1 to read from a closed file...\n");
	char buffer;
	int val = tfs_readByte(fd1, &buffer);
	if(val < 0) {
		if(val == EFILOP)
			fprintf(stderr, "File is not opened.\n");
		else {
			fprintf(stderr, "Some other file error: %d\n", val);
		}
	}
	fprintf(stderr, "Attempt #2 to read from a closed file...\n");
	char* buf = "hello :)";
	// Write on a closed file
		val = tfs_writeFile(fd1, buf, strlen(buf));
		if(val < 0) {
			if(val == EFILOP)
				fprintf(stderr, "File is not opened.\n");
			else {
				fprintf(stderr, "Some other file error: %d\n", val);
			}
		}

	// Open files
	fprintf(stderr, "\nOpening files 'one', 'two', and 'three'...\n");
	fd1 = tfs_openFile("one");
	if(fd1 < 0){
		fprintf(stderr, "File error: %d\n", fd1);
		return fd1;
	}
	fileDescriptor fd2 = tfs_openFile("two");
	if(fd2 < 0){
		fprintf(stderr, "File error: %d\n", fd2);
		return fd2;
	}
	fileDescriptor fd3 = tfs_openFile("three");
	if(fd3 < 0){
		fprintf(stderr, "File error: %d\n", fd3);
		return fd3;
	}

	// Generate string that takes up 2 blocks
	char *b = (char *) malloc(sizeof(char) * 504);
	for(int i = 0; i < 252; i++) {
		b[i] = 'a';
	}
	for(int i = 252; i < 450; i++) {
		b[i] = 'b';
	}
	b[450] = '\0';

	// Write to files
	fprintf(stderr, "\nWriting to all three files...\n");
	int write_val;
	write_val = tfs_writeFile(fd1,b,504);
	if(write_val < 0) {
		fprintf(stderr, "writeFile() error: %d\n", write_val);
		return write_val;
	}
	write_val = tfs_writeFile(fd2,"writing to two",252);
	if(write_val < 0) {
		fprintf(stderr, "writeFile() error: %d\n", write_val);
		return write_val;
	}
	write_val = tfs_writeFile(fd3,"writing to three",252);
	if(write_val < 0) {
		fprintf(stderr, "writeFile() error: %d\n", write_val);
		return write_val;
	}

	// Read from a file
	fprintf(stderr, "\nReading from file 'two'...\n");
	val = tfs_readByte(fd2, &buffer);
	fprintf(stderr, "Byte 0: %c\n", buffer);
	tfs_seek(fd1, 5);
	val = tfs_readByte(fd2, &buffer);
	fprintf(stderr, "Byte 5: %c\n", buffer);

	// Attemping to seek outside of file bounds
	fprintf(stderr, "Attempting to seek and read outside of file bounds. Seek offset: %d\n", 1036);
	tfs_seek(fd1, 1036);
	val = tfs_readByte(fd1, &buffer);
	if(val == EEOF)
		fprintf(stderr, "Cannot read past end of file.\n");
	else 
	fprintf(stderr, "Byte 1036: %c\n", buffer);

	// Rename a file
	fprintf(stderr, "\nRenaming file 'three' to 'four...\n");
	tfs_readdir();
	tfs_rename(fd3,"four");
	tfs_readdir();

	// Delete a file
	fprintf(stderr, "\nDeleting file 'one'...\n");
	tfs_readdir();
	val = tfs_deleteFile(fd1);
	if(val < 0) {
		fprintf(stderr, "Error deleting file: %d\n", val);
		return val;
	}
	fprintf(stderr, "File 'one' deleted.\n");
	tfs_readdir();

	// Print creation date of file
    fprintf(stderr, "\nCreation date of file 'two': %d\n",tfs_readFileInfo(fd2));
	fprintf(stderr, "\nClosing all files...\n");
	tfs_closeFile(fd1);
	tfs_closeFile(fd2);
	tfs_closeFile(fd3);
	fprintf(stderr, "Files closed.\n");

	free(b);

	// Unmount file system
	tfs_unmount();
	
}
