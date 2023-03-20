#include <stdio.h>
#include <string.h>
#include "tinyFS.h"
#include "TinyFS_errno.h"

#include "libDisk.h"

#include "libTinyFS.h"
#include "tfs_file.c"
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




/* Implementation of tinyFS filesystem related APIs */
int tfs_mkfs(const char *filename, size_t nBytes){\
    fprintf(stderr, "Creating file system...\n");
    if(nBytes==0){
        nBytes=DEFAULT_DISK_SIZE;
    }
    
    int fs = openDisk(filename,nBytes);
    if (fs<0){
        return fs;
    }
    if(nBytes%BLOCKSIZE!=0){
        nBytes=nBytes-nBytes%BLOCKSIZE;
    }

    char *ini=(char*)malloc(256*sizeof(char));
    //superblock
    ini[0]=0x01;//type
    ini[1]=0x44;//magic num
    ini[2]=0x00;//next inode
    //next free block
    if(nBytes/BLOCKSIZE==1){
        ini[3]=0x00;
    }
    else{
        ini[3]=0x01;    
    }
    ini[4]=0x00;//tail inode
    ini[5]= (nBytes/BLOCKSIZE)-1;//tail free block
    ini[6]=nBytes/BLOCKSIZE;//number of blocks in fs
    ini[7]=(nBytes/BLOCKSIZE)-1; //number of free blocks
    writeBlock(fs,0,ini);
    memset(ini, 0, 256);
    //init free blocks
    if(nBytes>BLOCKSIZE){
        ini[0]=0x04;
        ini[1]=0x44;
        ini[2]=0x02;
        ini[3]=0x00;
        ini[4]=0x00;
        for (int i = 1;i< nBytes/BLOCKSIZE;i++){
            if(i==(nBytes/BLOCKSIZE)-1){
                ini[2]=0x00;
            }
            writeBlock(fs,i,ini);
            ini[2]=ini[2]+0x01;
            
        }
    }
    free(ini);
    closeDisk(fs);
    fprintf(stderr, "File system created.\n");
    return 0;
}
int tfs_mount(const char *diskname) 
{
    fprintf(stderr, "Mounting file system...\n");
    // if already mounted
    if(mounted >= 0){
        fprintf(stderr, "File system already mounted.\n");
        return EFSMT;
    }
    
    char *currblock=(char*)malloc(256*sizeof(char));
    int c = 0;
    mounted=openDisk(diskname,0);
    if(mounted < 0) {
        fprintf(stderr, "Unable to mount. Reason: Disk error.\n");
        return mounted;
    }
    
    int res = readBlock(mounted,c++,currblock);

    // check magic number
    while(currblock[1]==0x44 && res == 0){
        res = readBlock(mounted,c++,currblock);
    }
    if(currblock[1]==0x44 && res == EDSKBD){
        fprintf(stderr, "Mounted.\n");
        return 0;
    }
    else{
        // invalid file system
        fprintf(stderr, "Unable to mount. Reason: Invalid file system format.\n");
        mounted = EFSIV;
        return mounted;
    }

}

int tfs_unmount(void){
    fprintf(stderr, "Unmounting file system...\n");
    // if not mounted
    if (mounted < 0) {
        fprintf(stderr, "No file system to unmount.\n");
        return EFSNMT;
    }

    closeDisk(mounted);
    mounted = -1;
    fprintf(stderr, "File system unmounted.\n");
    return 0;
}
