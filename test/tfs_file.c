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
unsigned char** files;

fileDescriptor tfs_openFile(const char *name){
    if(strlen(name)>8){
        fprintf(stderr,"Error filename too big");
        return -1;
    }
    if(filecount==0){
        files = (void *)malloc( sizeof(unsigned char*));
        
    }
    else{
        unsigned char** reallocf = realloc(files, (filecount+1) * sizeof(unsigned char*));
        if (reallocf)
        {
            files = reallocf;
        }
        else{
            return -1;
        }
    }
    char * currblock = (char*)malloc(256*sizeof(char));
    unsigned char * superblock = (unsigned char*)malloc(256*sizeof(char));
    readBlock(mounted,0,superblock);
    char firstfree=superblock[3];
    char firstinode=superblock[2];
    bool found = false;
    if(firstinode!=0){
        
        while(readBlock(mounted,firstinode,currblock)==0){
            
            if(currblock[0]!=0x02){
                fprintf(stderr,"block indicator: %d\n",currblock[0]);
                break;
            }
            else if(strcmp(currblock+4,name)==0){
                files[filecount++]=(unsigned char *)currblock;
                found = true;
                break;
            }
            
                firstinode=currblock[2];
                if (firstinode==0){
                    break;
                }
            
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
            currblock[15]=0x00;//file ptr
            writeBlock(mounted,firstfree,currblock);
            files[filecount++]=(unsigned char *)currblock;

            readBlock(mounted,0,superblock);
            //if superblock does not have next inode (no inodes yet)
            //then set the inode
            if(superblock[2]==0){
                superblock[2]=firstfree;
            }
            //next free block
            superblock[3]=nextfree;

            //set next of tailnode
            if(superblock[4]!=0x00){
                unsigned char * tailinode;
                for(int j=0;j<filecount;j++){
                    if(files[j][3]==superblock[4]){
                        tailinode=files[j];
                    }
                }
                tailinode[2]=firstfree;
                writeBlock(mounted,superblock[4],tailinode);
                
            }
            
            //update superblock's tailnode
            superblock[4]=firstfree;
            superblock[7]-=0x01;
            writeBlock(mounted,0,superblock);
            
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
    unsigned char * superblock = (unsigned char*)malloc(256*sizeof(char));
    readBlock(mounted,0,superblock);
    if(fd>filecount || fd<0){
        return -1;

    }
    unsigned char * currnode=files[fd];
    if(currnode==NULL || fd>filecount || fd<0){
        return -1;
    }

    if(superblock[7]+currnode[13]<strlen(buffer)/(BLOCKSIZE-4)){
        fprintf(stderr,"Not enough space in disk");
        free(superblock);
        return -1;
    }
    currnode[15]=0x00; // set file ptr to zero

    
    char rootdata = currnode[14];//root data block of the inode
    char * currdata = (char*)malloc(256*sizeof(char));
    char freetail=superblock[5];
    unsigned char * lastfree =(unsigned char*)malloc(256*sizeof(char));
    char newtail=freetail;

    
    if(rootdata!=0x00){
        if(superblock[3]==0){
            superblock[3]=rootdata;
        }
        
    while(readBlock(mounted,rootdata,currdata)==0){
        
        superblock[7]+=0x01;
        //change free block to current block
        readBlock(mounted,newtail,lastfree);
        lastfree[2]=rootdata; 
        newtail=rootdata;
        rootdata=currdata[2];
        writeBlock(mounted,newtail,lastfree);
        //free block template
        memset(currdata,0,256);
        currdata[0]=0x04;
        currdata[1]=0x44;
        currdata[2]=rootdata;
        writeBlock(mounted,newtail,currdata);//turn data block into free block

        if(rootdata==0x00){
            break;
        }
    }
    superblock[5]=newtail;
    
    
    } 

    char firstfree=superblock[3];
    int writeptr=0;
    int endptr=252;
    char nextfree=firstfree;
    int blocks=size;
    
    if(blocks!=0){
        if(blocks%252!=0){
            blocks=(blocks+(252-blocks%252));
        }
    blocks=blocks/252;
    for (int i=0; i<blocks;i++){
        currnode[13]+=1;

        superblock[7]-=0x01;
        if(superblock[7]==0x00){
            superblock[5]=0x00;
        }
        readBlock(mounted,nextfree,currdata);
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
        if(writeptr>strlen(buffer)){
            memset((currdata+4),0,252);
        }
        else{
            strncpy((currdata+4),buffer+writeptr,endptr);
            writeptr+=252;
            endptr+=252;
        }
        
        writeBlock(mounted,firstfree,currdata);
        firstfree=nextfree;
        
    }
    currnode[14]=superblock[3];
    superblock[3]=nextfree;
    }
    
    
    writeBlock(mounted,0,superblock);
    
    writeBlock(mounted,currnode[3],currnode);
    free(superblock);
    free(currdata);
    free(lastfree);
    return 0;
}
int tfs_deleteFile(fileDescriptor FD){
    unsigned char * superblock =(unsigned char*)malloc(256*sizeof(char));
    readBlock(mounted,0,superblock);
    char freetail=superblock[5];
    unsigned char * lastfree =(unsigned char*)malloc(256*sizeof(char));
    
    char newtail=freetail;
    if(FD>filecount || FD<0){
        return -1;

    }
    unsigned char * del = files[FD];
    if(del==NULL || FD>filecount || FD<0){
        return -1;
    }
    unsigned char * currdata = (unsigned char*)malloc(256*sizeof(char));
    char rootdata=del[14];
    if(superblock[2]==del[3]){
        superblock[2]=del[2];
    }
    superblock[7]+=0x01; 
    if(rootdata!=0x00){
        if(superblock[3]==0){
            superblock[3]=rootdata;
        }
       
    while(readBlock(mounted,rootdata,currdata)==0){
        
        superblock[7]+=0x01;
        //change free block to current block
        readBlock(mounted,newtail,lastfree);
        lastfree[2]=rootdata; 
        
        rootdata=currdata[2];
        writeBlock(mounted,newtail,lastfree);
        newtail=lastfree[2];
        //free block template
        char dataptr=currdata[2];
        memset(currdata,0,256);
        currdata[0]=0x04;
        currdata[1]=0x44;
        if(dataptr==0x00){
            currdata[2]=del[3];
        }
        else{
            currdata[2]=rootdata;

        }
        writeBlock(mounted,newtail,currdata);//turn block into free block

        if(rootdata==0x00){
            break;
        }
    }
    }
    superblock[5]=del[3];
    unsigned char * prev;
    for (int j =0;j<filecount;j++){
        if(files[j][2]==del[3]){
            prev=files[j];
        }
    }
    prev[2]=del[2];
    if(superblock[4]==del[3]){
        superblock[4]=prev[3];
    }
    writeBlock(mounted,prev[3],prev);
    if(superblock[4]!=0x00){
                unsigned char * tailinode;
                for(int j=0;j<filecount;j++){
                    if(files[j][3]==superblock[4]){
                        tailinode=files[j];
                    }
                }
                tailinode[2]=0x00;
                writeBlock(mounted,superblock[4],tailinode);
                
            }
            
            //update superblock's tailnode
    char inode=del[3];
    memset(del,0,256);
    del[0]=0x04;
    del[1]=0x44;
    writeBlock(mounted,inode,del);
    writeBlock(mounted,0,superblock);
    free(superblock);
    free(del);
    files[FD]=NULL;
    
    
    
    return 0;
    

}
int tfs_readByte(fileDescriptor FD, char *buffer){
    if(FD>filecount || FD<0){
        return -1;

    }
    unsigned char * r = files[FD];
    if(r==NULL){
        return -1;
    }
    int ptr=r[15];
    int block= ptr;
    // int loc = block%251;
    if(block!=0){
        if(block%251!=0){
            block=(block+(251-block%251));
        }
    block=(block/251);
    }
    fprintf(stderr,"\n offset: %d data block: %d\n",ptr,block);
    return 0;
}

int tfs_seek(fileDescriptor fd, off_t offset){
    if(fd>filecount || fd<0 || offset<0){
        return -1;

    }
    unsigned char * r = files[fd];
    if(r==NULL || offset>(r[13]*252)-1){
        return -1;
    }
    unsigned int o = offset;
    r[15]=o%252;
    int n = (o/252)+1;
    char * datablock = (char* )malloc(sizeof(char)*256);
    while(n>0 && readBlock(mounted,r[2],datablock)==0){
        n-=1;
        
    }

    
    fprintf(stderr,"\n n: %d offset: %d block: %d",n, o%252,r[16]);

    writeBlock(mounted,r[3],r);
    return 0;
}