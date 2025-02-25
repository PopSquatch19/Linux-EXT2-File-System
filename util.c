/*********** util.c file ****************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern MTABLE mtable[NMTABLE];

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  int i;
  char *s; 
  //printf("tokenize %s\n", pathname);    //commented 11/29/22

  strcpy(gpath, pathname);   // tokens are in global gpath[ ]
  n = 0;

  s = strtok(gpath, "/");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }
  name[n] = 0;
  
  //for (i= 0; i<n; i++)
    //printf("%s  ", name[i]);  //commented 11/29/22
  //printf("\n");
}

int link_tokenize(char *pathname)
{
  int i;
  char *s;
  printf("tokenize %s\n", pathname);

  strcpy(gpath, pathname);   // tokens are in global gpath[ ]
  n = 0;

  s = strtok(gpath, " ");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, " ");
  }
  name[n] = 0;
  //printf("exectued?\n");
  for (i= 0; i<n; i++)
    printf("%s  ", name[i]);
  printf("\n");
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, offset;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount && mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino into buf[ ]    
       blk    = (ino-1)/8 + iblk;
       offset = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

       get_block(dev, blk, buf);    // buf[ ] contains this INODE
       ip = (INODE *)buf + offset;  // this INODE in buf[ ] 
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
  return 0;
}

void iput(MINODE *mip)  // iput(): release a minode
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE *ip;

 if (mip==0) 
     return;

 mip->refCount--;
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;

 // write INODE back to disk
 block = (mip->ino - 1) / 8 + iblk; //was originally 'iblock"
 offset = (mip->ino - 1) % 8;

 // get block containing this inode
 get_block(mip->dev, block, buf);
 ip = (INODE *)buf + offset; // ip points at INODE
 *ip = mip->INODE; // copy INODE to inode in block
 put_block(mip->dev, block, buf); // write back to disk
 midalloc(mip); // mip->refCount = 0;
 
 //>>>>>>> 03fdacc0835f6ab26799bf182d021285205ec557
 /* write INODE back to disk */
 block = (mip->ino - 1);
 offset = (mip->ino) % 8;

 //get block containing ino

 get_block(mip->dev, block, buf);
 ip = (INODE *)buf + offset;
 *ip = mip->INODE;
 put_block(mip->dev, block, buf);
 //midalloc(mip);
 /**************** NOTE ******************************
  For mountroot, we never MODIFY any loaded INODE
                 so no need to write it back
  FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

  Write YOUR code here to write INODE back to disk
 *****************************************************/
}

int midalloc(MINODE *mip){
  mip->refCount = 0;
}

int search(MINODE *mip, char *name)
{
   //int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

  //printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);   
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/
   for(int i = 0; i < 12; i++){
   get_block(dev, ip->i_block[i], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   if(dp->rec_len == 0){
    return 0;
   }
   //printf("  ino   rlen  nlen  name\n");`//commented 11/29/22

   while (cp < sbuf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len); // dp->name is NOT a string
     temp[dp->name_len] = 0;                // temp is a STRING
     //printf("%4d  %4d  %4d    %s\n",  
	    //dp->inode, dp->rec_len, dp->name_len, temp); // print temp !!!    //commented 11/29/22

     if (strcmp(temp, name)==0){            // compare name with temp !!!
        //printf("found %s : ino = %d\n", temp, dp->inode);   //commented 11/29/22
        return dp->inode;
     }

     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   }
   return 0;
}

int getino(char *pathname) // return ino of pathname   
{
  int i, ino, blk, offset;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  //printf("\ngetino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;
  
  // starting mip = root OR CWD
  if (pathname[0]=='/')
     mip = root;
  else
     mip = running->cwd;

  mip->refCount++;         // because we iput(mip) later
  
  tokenize(pathname);

  for (i=0; i<n; i++){
      //printf("===========================================\n"); //commented 11/29/22
      //printf("getino: i=%d name[%d]=%s\n", i, i, name[i]); //commented 11/29/22
 
      ino = search(mip, name[i]);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }

      iput(mip);
      mip = iget(dev, ino);
   }

   iput(mip);
   return ino;
}

// These 2 functions are needed for pwd()
//int findmyname(MINODE *parent, u32 myino, char myname[ ])
int findmyname(MINODE *parent, u32 myino, char *myname)
{
  // WRITE YOUR code here
   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   // printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);
    ip = &(parent->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   //get_block(dev, myino, sbuf);
   get_block(dev, parent->INODE.i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   printf("  ino   rlen  nlen  name\n");

   //cp = sbuf + BLKSIZE; This stops the infinite loop for debugging purposes
   while (cp < sbuf + BLKSIZE){
     //strncpy(temp, myname,strlen(myname)); // dp->name is NOT a string  dp->name_len
     strncpy(temp, dp->name, strlen(dp->name));
     temp[dp->name_len] = 0;                // temp is a STRING
     printf("%4d  %4d  %4d    %s\n", 
     dp->inode, dp->rec_len, dp->name_len, temp); // print temp !!!

     if (dp->inode == myino){            // compare name with temp !!!
        printf("found %s : myname: %s\n", temp, myname);
       //printf("found %s\n", temp);
	      strcpy(myname, temp);
	      //printf("test\n");
        return 1;
     }

     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   
   return 0;


  // search parent's data block for myino; SAME as search() but by myino
  // copy its name STRING to myname[ ]
}

int findino(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
//int findino(MINODE *mip)
{
  DIR *dp;
  char *cp, sbuf[1024];
  get_block(dev, mip->INODE.i_block[0], sbuf);
  dp = (DIR *)sbuf;
  cp = sbuf;
  *myino = dp->inode;
  cp += dp->rec_len;
  dp = (DIR *)cp;
  //printf("myino = %4d dpino = %4d\n", *myino, dp->inode);
  return dp->inode;
  //return 2;
}

MTABLE * get_mtable(int dev){
  return &mtable;
}
