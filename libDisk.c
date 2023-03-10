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
            return 0;
        }
    }

    if(nBytes<BLOCKSIZE){
        return -1;
    }
    if(nBytes%BLOCKSIZE!=0){
        nBytes=nBytes-nBytes%BLOCKSIZE;
    }
    disk = fopen(filename, "wb+");
    int z =0;
    for (int i = 0; i<nBytes;i++){
        fwrite(&z,1,1,disk);
        z++;
    }
    printf("New disk address: %p\n",disk);
    disks[diskCount]=disk;
    printf("Address at %d: %p\n",diskCount,disks[diskCount]);
    diskCount++;
}

int closeDisk(int disk){
    fclose(disks[disk]);
    disks[disk]=NULL;
    
}

int readBlock(int disk, int bNum, void *block){
    printf("reading from disk: %d\n",disk);
    FILE* d = disks[disk];
    fseek(d, bNum*BLOCKSIZE, SEEK_SET);
    fread(block,1,256,d);
    fseek(d, 0, SEEK_SET);
    return 0;
}
int writeBlock(int disk, int bNum, const void *block){
    printf("writing to disk: %d\n",disk);
    FILE* d = disks[disk];
    //find file size
    fseek(d, 0, SEEK_END);
    int sz = ftell(d);
    fseek(d, 0, SEEK_SET);
    printf("FILE SIZE: %d\n",sz);
    if(bNum*BLOCKSIZE>sz){
        return -1;
    }
    else{
        fseek(d, bNum*BLOCKSIZE, SEEK_SET);
        fwrite(block,1,256,d);
        fseek(d, 0, SEEK_SET);
        return 0;
    }
}
int main(){
    fprintf(stderr,"%d",sizeof(FILE*));
    void * str = (void*) malloc(BLOCKSIZE);
    openDisk("./disk",256);
    openDisk("./disk1",256);
    readBlock(0,0,str);
    for (int i=0;i<BLOCKSIZE;i++){
        printf("%01x\n", ((uint8_t*) str)[i]);
    }
    printf("\n");
    int z = 2;
    writeBlock(0,0,&z);
    readBlock(0,0,str);
    // for (int i=0;i<BLOCKSIZE;i++){
    //     printf("%01x\n", ((uint8_t*) str)[i]);
    // }
}
