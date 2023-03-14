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




/* Implementation of tinyFS filesystem related APIs */
int tfs_mkfs(const char *filename, size_t nBytes){\
    if(nBytes==0){
        nBytes=DEFAULT_DISK_SIZE;
    }
    int fs = openDisk(filename,nBytes);
    if (fs<0){
        return -1;
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
    ini[5]=nBytes/BLOCKSIZE;//number of bytes in fs
    memset(ini+6, 0, 250);
    writeBlock(fs,0,ini);
    //free blocks
    if(nBytes>BLOCKSIZE){
        ini[0]=0x04;
        ini[1]=0x44;
        ini[2]=0x02;
        ini[3]=0x00;
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
    return 0;
}
int tfs_mount(const char *diskname) 
{
    if(mounted!=-1){
        return -2;
    }
    char *currblock=(char*)malloc(256*sizeof(char));
    int c = 0;
    mounted=openDisk(diskname,0);
    int res = readBlock(mounted,c++,currblock);
    while(currblock[1]==0x44 && res == 0){
        res = readBlock(mounted,c++,currblock);
    }
    if(currblock[1]==0x44 && res == -2){
        return 0;
    }
    else{
        fprintf(stderr,"res :%d\n",res);
        fprintf(stderr,"currblock :%d\n",currblock[1]);
        return -1;
    }

}
int tfs_unmount(void){
    closeDisk(mounted);
    mounted = -1;
    return 0;
}
