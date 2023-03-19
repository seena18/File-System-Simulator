#include "tinyFS.h"
#include "TinyFS_errno.h"
#include "libDisk.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <stdint.h>
#define BLOCKSIZE 256

int openDisk(const char *filename, int nBytes);
int closeDisk(int disk);
int readBlock(int disk, int bNum, void *block);
int writeBlock(int disk, int bNum, const void *block);

int diskCount=0;
FILE* disk;
FILE** disks;

int openDisk(const char *filename, int nBytes){
    
    if(diskCount==0){
        disks = (void *)malloc( sizeof(FILE*));
        
    }
    else{
        FILE** reallocd = realloc(disks, (diskCount+1) * sizeof(FILE*));
        if (reallocd)
        {
            disks = reallocd;
        }
        else{
            // allocation error
            return EALLOC;
        }
    }
    if(nBytes==0){
        disk = fopen(filename, "rb+");
        // file error
        if (disk==NULL){
            return ENODSK;
        }
        else{
            disks[diskCount]=disk;
            // fprintf(stderr, "diskCount: %d , disks[diskCount]: %p\n",diskCount,disks[diskCount]);
            diskCount++;
            return diskCount-1;
        }
    }
    // block must be at least size 1
    if(nBytes<BLOCKSIZE){
        return EBLKSZ;
    }
    if(nBytes%BLOCKSIZE!=0){
        nBytes=nBytes-nBytes%BLOCKSIZE;
    }
    disk = fopen(filename, "wb+");
    int z =0;
    for (int i = 0; i<nBytes;i++){
        fwrite(&z,1,1,disk);
    }
    disks[diskCount]=disk;
    diskCount++;
    return diskCount-1;
}

int closeDisk(int disk){
    fclose(disks[disk]);
    disks[disk]=NULL;
    return 0;
}

int readBlock(int disk, int bNum, void *block){
    // fprintf(stderr, "disk: %d , disks[disk]: %p\n",disk,disks[disk]);
    if(disks[disk]==NULL){
        return EDSKOP;
    }
    
    
    FILE* d = disks[disk];
    fseek(d, 0, SEEK_END);
    int sz = ftell(d);
    fseek(d, 0, SEEK_SET);
    // out of disk bounds
    if((bNum+1)*BLOCKSIZE>sz){
        return EDSKBD;
    }
    fseek(d, bNum*BLOCKSIZE, SEEK_SET);
    fread(block,1,256,d);
    fseek(d, 0, SEEK_SET);
    return 0;
}
int writeBlock(int disk, int bNum, const void *block){
    
    FILE* d = disks[disk];
    // find file size
    fseek(d, 0, SEEK_END);
    int sz = ftell(d);
    fseek(d, 0, SEEK_SET);

    if((bNum+1)*BLOCKSIZE>sz){
        // out of disk bounds
        return EDSKBD;
    }
    else{
        fseek(d, bNum*BLOCKSIZE, SEEK_SET);
        fwrite(block,1,256,d);
        fseek(d, 0, SEEK_SET);
        return 0;
    }
}
