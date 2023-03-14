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
    if(strlen(name)>8){
        fprintf(stderr,"Error filename too big");
        return -1;
    }
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
            currblock[3]=firstfree;//current block number
            strcpy((currblock+4),name);//filename start
            currblock[13]=0x01;//size of file
            currblock[14]=0x00;//root data block
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
            currblock[7]-=0x01;
            writeBlock(mounted,0,currblock);
            files[filecount++]=currblock;
        }
    }
    return filecount-1;
}
int tfs_closeFile(fileDescriptor fd){
    free(files[fd]);
    files[fd]=NULL;
    return 0;
}
int tfs_writeFile(fileDescriptor fd, const char *buffer, size_t size){
    char * superblock = (char*)malloc(256*sizeof(char));
    readBlock(mounted,0,superblock);
    char * currnode=files[fd];
    if(superblock[7]+currnode[13]<strlen(buffer)/(BLOCKSIZE-4)){
        fprintf(stderr,"Not enough space in disk");
        free(superblock);
        return -1;
    }


    
    char rootdata = currnode[14];//root data block of the inode
    char * currdata = (char*)malloc(256*sizeof(char));
    char freetail=superblock[5];
    char * lastfree =(char*)malloc(256*sizeof(char));
    char newtail=freetail;
    currnode[14]=freetail;
    if(rootdata!=0x00){
    while(readBlock(mounted,rootdata,currdata)){
        superblock[7]+=0x01;
        //change free block to current block
        readBlock(mounted,newtail,lastfree);
        lastfree[2]=rootdata; 
        newtail=rootdata;
        rootdata=currdata[2];
        
        //free block template
        memset(currdata,0,256);
        currdata[0]=0x04;
        currdata[1]=0x44;
        writeBlock(mounted,rootdata,currdata);//turn data block into free block

        if(currdata[2]==0x00){
            break;
        }
    }
    superblock[5]=newtail;
    
    }

    char firstfree=superblock[3];
    int writeptr=0;
    int endptr=252;
    char nextfree=firstfree;
    int blocks=strlen(buffer);
    blocks=(blocks+(252-blocks%252))/252;
    for (int i=0; i<blocks;i++){
        superblock[7]-=0x01;
        readBlock(mounted,firstfree,currdata);
        nextfree=currdata[2];
        currdata=memset(currdata, 0, 256);
        currdata[0]=0x03;//block type
        currdata[1]=0x44;//magic num
        if(i!=blocks-1){
            currdata[2]=nextfree;
        }
        else{
            currdata[2]=0x00;
        }
        //next data
        strncpy((currdata+4),buffer+writeptr,endptr);
        writeptr+=252;
        endptr+=252;
        writeBlock(mounted,firstfree,currdata);
        firstfree=nextfree;
        
    }
    superblock[3]=nextfree;
    writeBlock(mounted,0,superblock);
    fprintf(stderr,"test: %d",currnode[3]);

    free(superblock);
    free(currdata);
    free(lastfree);
    return 0;
}
