#include <stdio.h>
#include <string.h>
#include "tinyFS.h"
#include "TinyFS_errno.h"

#include "libDisk.h"

#include "libTinyFS.h"
#include "tfs_internal.h"
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
int fss;

int mount = -1;

/* Implementation of tinyFS filesystem related APIs */
int tfs_mkfs(const char *filename, size_t nBytes){
    int fs = openDisk(filename,nBytes);
    if (fs<0){
        return -1;
    }
    if(nBytes%BLOCKSIZE!=0){
        nBytes=nBytes-nBytes%BLOCKSIZE;
    }

    char *ini=(char*)malloc(256*sizeof(char));
    //superblock
    ini[0]=0x01;
    ini[1]=0x44;
    memset(ini+3, 0, 253);
    writeBlock(fs,0,ini);
    //free blocks
    if(nBytes/BLOCKSIZE>BLOCKSIZE){
        ini[0]=0x04;
        ini[1]=0x44;
        for (int i = 1;i< nBytes/BLOCKSIZE;i++){
            writeBlock(fs,i,ini);
        }
    }
    free(ini);
    closeDisk(fs);
    return 0;
}
int tfs_mount(const char *diskname) 
{
    if(mount!=-1){
        return -2;
    }
    char *currblock=(char*)malloc(256*sizeof(char));
    int c = 0;
    mount=openDisk(diskname,0);
    int res = readBlock(mount,c++,currblock);
    while(currblock[1]==0x44 && res == 0){
        res = readBlock(mount,c++,currblock);
    }
    if(currblock[1]==0x44 && res == -2){
        return 0;
    }
    else{
        return -1;
    }

}
int tfs_unmount(void){
    closeDisk(mount);
    mount = -1;
    return 0;
}
int main(){
    tfs_mkfs("disk1.dsk",256);
    int r = tfs_mount("disk1.dsk");
    fprintf(stderr,"Mount result: %d\n",r);
    fprintf(stderr,"Mounted: %d\n",mount);
}