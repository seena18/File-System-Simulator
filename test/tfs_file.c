#include <stdio.h>

#include "tinyFS.h"
#include "TinyFS_errno.h"

#include "libDisk.h"

#include "libTinyFS.h"
#include "tfs_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <stdint.h>
#include <ctype.h>

/* Implementation of tinyFS file related APIs */


void printBlock(unsigned char * b){
    for(int j = 0; j<256;j++){
        fprintf(stderr,"%d ",b[j]);
    }
}
fileDescriptor tfs_openFile(const char *name){
    gettimeofday(&tv,NULL);

    // file name empty or too long
    if(strlen(name)>8 || strlen(name) < 1){
        fprintf(stderr,"Error: file name must be between 1-8 characters long.\n");
        return EFILNM;
    }
    // file name not alphanumeric
    for(int i = 0; i < strlen(name); i++) {
        if(!isalnum(name[i])){
            fprintf(stderr, "File name must contain alphanumeric characters.\n");
            return EFILNM;
        }
    }

    if(filecount==0){
        files = (void *)malloc( sizeof(unsigned char*));
        offset_table = (int *)malloc(sizeof(int));
    }
    else{
        unsigned char** reallocf = realloc(files, (filecount+1) * sizeof(unsigned char*));
        int* realloc_offset = realloc(offset_table, (filecount+1) * sizeof(int));
        if (reallocf && realloc_offset)
        {
            files = reallocf;
            offset_table = realloc_offset;
        }
        else{
            // allocation error
            return EALLOC;
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
                memcpy((currblock+24),(void *) &(tv.tv_sec),8);
                files[filecount++]=(unsigned char *)currblock;
                writeBlock(mounted,currblock[3],currblock);
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
            memcpy((currblock+16),(void *) &(tv.tv_sec),8);//creation time
            memcpy((currblock+24),(void *) &(tv.tv_sec),8);//access time
            memcpy((currblock+32),(void *) &(tv.tv_sec),8);//modify time
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

    offset_table[filecount - 1] = 0;
    return filecount-1;
}
int tfs_closeFile(fileDescriptor fd){
    free(files[fd]);
    files[fd]=NULL;
    offset_table[fd]=-1;
    return 0;
}
int tfs_writeFile(fileDescriptor fd, const char *buffer, size_t size){

    gettimeofday(&tv,NULL);

    unsigned char * superblock = (unsigned char*)malloc(256*sizeof(char));
    readBlock(mounted,0,superblock);
    if(fd>filecount || fd<0){
        return EFILOP;

    }
    unsigned char * currnode=files[fd];
    if(currnode==NULL || fd>filecount || fd<0){
        return EFILOP;
    }

    if(superblock[7]+currnode[13]<strlen(buffer)/(BLOCKSIZE-4)){
        fprintf(stderr,"Not enough space in disk");
        free(superblock);
        return EDSKBD;
    }
    offset_table[fd]=0x00; // set file ptr to zero

    
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
    memcpy((currnode+32),(void *) &(tv.tv_sec),8);//modify time
    writeBlock(mounted,currnode[3],currnode);
    // free(superblock);
    // free(currdata);
    // free(lastfree);
    return 0;
}
int tfs_deleteFile(fileDescriptor fd){
    unsigned char * superblock =(unsigned char*)malloc(256*sizeof(char));
    readBlock(mounted,0,superblock);
    char freetail=superblock[5];
    unsigned char * lastfree =(unsigned char*)malloc(256*sizeof(char));
    
    char newtail=freetail;
    if(fd>filecount || fd<0){
        return EFDIV;

    }
    unsigned char * del = files[fd];
    if(del==NULL || fd>filecount || fd<0){
        return EFDIV;
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
    
        if(files[j]!=NULL && files[j][2]==del[3]){
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
                    if(files[j]!=NULL && files[j][3]==superblock[4]){
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
    files[fd]=NULL;
    offset_table[fd]=-1;
    
    return 0;
}

int tfs_readByte(fileDescriptor fd, char *buffer){

    
    gettimeofday(&tv,NULL);

    if(fd>filecount || fd<0){
        return EFILOP;

    }
    unsigned char * r = files[fd];
    if(r==NULL){
        return EFILOP;
    }
    fprintf(stderr, "offset_table[fd]: %d\n", offset_table[fd]);
    fprintf(stderr, "max size: %d\n", 252*(r[13]-1) - 1);
    if(offset_table[fd] > 252*(r[13]-1) - 1){
        return EEOF;
    }
    int block_offset=offset_table[fd]%252;
    int n_data_block = (offset_table[fd]/252)+1;
    int block_addr = 0;
    char addr=r[14];   
    char * datablock = (char* )malloc(sizeof(char)*256);
    while(addr!=0 && readBlock(mounted,addr,datablock)==0){
        n_data_block-=1;
        if(n_data_block==0){
            block_addr=addr;
        }
        addr=datablock[2];
    }

    int ptr=block_offset;
    int block = block_addr;
    //fprintf(stderr,"\n ptr: %d block: %d",ptr+4,block);
    readBlock(mounted,block,datablock);
    *buffer=datablock[ptr+4];
    
    offset_table[fd]+=1;

    memcpy((r+24),(void *) &(tv.tv_sec),8);//access time
    free(datablock);
    return 0;
}

int tfs_seek(fileDescriptor fd, off_t offset){
    if (fd>filecount || fd<0 || offset<0) {
        return EFILOP;

    }
    unsigned char * r = files[fd];
    if (r==NULL) {
        return EFILOP;
    }
    offset_table[fd]=offset;

    return 0;
}

int tfs_rename(fileDescriptor fd, const char * name){

    gettimeofday(&tv,NULL);

        // file name empty or too long
    if(strlen(name)>8 || strlen(name) < 1){
        fprintf(stderr,"Error: file name must be between 1-8 characters long.\n");
        return EFILNM;
    }
    // file name not alphanumeric
    for(int i = 0; i < strlen(name); i++) {
        if(!isalnum(name[i])){
            fprintf(stderr, "File name must contain alphanumeric characters.\n");
            return EFILNM;
        }
    }

    if (fd>filecount || fd<0) {
        return EFILNM;
        //error if file descriptor not valid

    }
    unsigned char * r = files[fd];
    if ( r==NULL) {
        return EFILOP; //error if file descriptor not valid
    }

    for(int i =0;i<filecount;i++){
        
        if(files[i]!=NULL){
            unsigned char* f =files[i];
            if(strcmp((char*)(f+4),name)==0){
                tfs_deleteFile(i);
            }
        }
    }
    strcpy((char*)(r+4),name);
    
    memcpy((r+32),(void *) &(tv.tv_sec),8);//modify time
    writeBlock(mounted,r[3],r);
    return 0;

}
void tfs_readdir(){
    unsigned char * superblock = (unsigned char*)malloc(256*sizeof(char));
    readBlock(mounted,0,superblock);
    char rootinode=superblock[2];
    unsigned char * currblock = (unsigned char*)malloc(256*sizeof(char));
    fprintf(stderr,"FILES\n");
    fprintf(stderr,"----------------------------\n");
    if(rootinode!=0x00){

    
    while(readBlock(mounted,rootinode,currblock)==0){
        fprintf(stderr,"%s\n",(currblock+4));
        rootinode=currblock[2];
        if(rootinode==0){
            break;
        }
    }
    fprintf(stderr,"----------------------------\n");
    }
    free(superblock);
    free(currblock);

    
}

int tfs_readFileInfo(fileDescriptor fd){
    if (fd>filecount || fd<0) {
        return EFILOP;
        //error if file descriptor not valid

    }
    unsigned char * r = files[fd];
    if ( r==NULL) {
        return EFILOP; //error if file descriptor not valid
    }
    int c;
    memcpy(&c,(r+16),8);
    return c;
 }

 