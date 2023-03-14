#include <stdio.h>

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

/* Implementation of tinyFS file related APIs */
int filecount=0;
char** files;
fileDescriptor tfs_openFile(const char *name){
    if(filecount==0){
        files = (void *)malloc( sizeof(char*));
        
    }
    else{
        char** reallocf = realloc(files, (filecount+1) * sizeof(char*));
        if (reallocf)
        {
            files = reallocf;
        }
        else{
            return -1;
        }
    }
    char * currblock = (char*)malloc(256*sizeof(char));
    readBlock(mounted,0,currblock);
    char firstfree=currblock[3];
    char firstinode=currblock[2];
    bool found = false;
    if(firstinode!=0){
        while(readBlock(mounted,firstinode,currblock)){
            if(currblock[0]!=0x02){
                fprintf(stderr,"block indicator: %d",currblock[0]);
            }
            else if(strcmp(currblock+4,name)==0){
                files[filecount++]=currblock;
                found = true;
                break;
            }
            firstinode=currblock[2];
        }
    }
    if(!found){
        if(firstfree!=0){
            readBlock(mounted,firstfree,currblock);
            char nextfree=currblock[2];
            currblock=memset(currblock, 0, 256);
            currblock[0]=0x02;//block type
            currblock[1]=0x44;//magic num
            currblock[2]=0x00;//next inode
            strcpy((currblock+4),name);//filename start
            writeBlock(mounted,firstfree,currblock);
            readBlock(mounted,0,currblock);
            //if superblock does not have next inode (no inodes yet)
            //then set the inode
            if(currblock[2]==0){
                currblock[2]=firstfree;
            }
            //next free block
            currblock[3]=nextfree;

            //set next of tailnode
            if(currblock[4]!=0x00){
                char * tailinode = (char*)malloc(256*sizeof(char));
                readBlock(mounted,currblock[4],tailinode);
                tailinode[2]=firstfree;
                writeBlock(mounted,currblock[4],tailinode);
                free(tailinode);
            }
            
            //update superblock's tailnode
            currblock[4]=firstfree;
            writeBlock(mounted,0,currblock);
            files[filecount++]=currblock;
        }
    }
    return filecount-1;
}
