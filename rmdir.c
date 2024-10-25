/************* rmdir.c file **************/

#ifndef __RMDIR_C__
#define __RMDIR_C__

#include "open_close.c"
//#include "alloc.c"

int last_entry(char *buf, int del_len)
{
  char *cp = buf;
  DIR  *dp = (DIR *)cp;

  //printf("last_entry del_len=%d\n", del_len);

  while (cp + dp->rec_len < buf + BLKSIZE){
     //printf("%d %d %d %s\n",dp->inode,dp->rec_len,dp->name_len,dp->name);
     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("last entry=[%d %d] ",  dp->inode, dp->rec_len);
  dp->rec_len += del_len;
  printf("last entry=[%d %d]\n", dp->inode, dp->rec_len);
  return 1;
} 

// remove child name from parent directory 
int rm_child(MINODE *pip, char *name)
{
  int i, j, del_len; 
  DIR  *dp, *pp;
  char *cp, *cq, c;  
  char buf[BLKSIZE];
  
  // consider parent DIR has only 1 data block
  get_block(pip->dev, pip->INODE.i_block[0], buf);
  pp = dp = (DIR *)buf;
  cp = buf;
      
  while (cp < buf + BLKSIZE){
     if (strncmp(dp->name, name, dp->name_len)==0){ // found child name
       del_len = dp->rec_len; // length to delete
       
       // ignore ONLY entry in block case
       
       // check if it's LAST entry
       cq = (char *)dp;
       if (cq + dp->rec_len >= buf + BLKSIZE){ // if LAST entry

         printf("LAST entry in block [%d %s] ", dp->rec_len, dp->name);
    printf("pp is [%d %s]\n", pp->rec_len, pp->name);
          printf("add delete len to PREVIOUS entry\n");
   
    pp->rec_len += dp->rec_len;
       }
       else{  // entry in middle of block
           last_entry(buf, del_len);
           memcpy(cp, cp+dp->rec_len, (&buf[BLKSIZE]-cp));
       }
       put_block(dev, pip->INODE.i_block[0], buf); // write parent block back
       return 1;                      // OK
     }
     pp = dp;                         // pp follows dp
     cp += dp->rec_len;               // advance cp;
     dp = (DIR *)cp;
  }
  return -1;                          // BAD
}

// int rm_child(MINODE *pip, char *name)
// {
//     int ino = search(pip, name);
//     MINODE *mip;
//     mip = iget(pip->dev, ino);
//     char buf[BLKSIZE], *cp;
//     int remain, prevlen, i = 0;
//     /*int c = 0;
//     while(c < 4){
//         printf("%d. %c\n", c, name[c]);
//         c++;
//     }*/
//     while(i < 12){
//         if(pip->INODE.i_block[i] != 0){
//         get_block(pip->dev, pip->INODE.i_block[i], buf);
//         cp = buf;
//         dp = (DIR *)cp;
//         //while((cp + dp->rec_len) < (buf + BLKSIZE)){
//         while(cp < buf + BLKSIZE){
//             //printf("%d. dp is currently %s and has ino %d\n", i, dp->name, dp->inode);
//             /*c = 0;
//             while(c < dp->name_len){
//                 printf("%d. %c\n", c, dp->name[c]);
//                 c++;
//             }*/
//             //printf("%s and %s have a difference of %d\n", dp->name, name, strcmp(dp->name, name));
//             if(!strncmp(dp->name, name, dp->name_len)){
//                 //printf("Found %s\n", dp->name);
//                 break;
//             }
//             prevlen = dp->rec_len;
//             cp += dp->rec_len;
// 	        dp = (DIR *)cp;
//             //}
//             remain = dp->rec_len;
//             /*if(dp->inode == ino){
//                 break;
//             }*/
//         }
//         }
//         if(dp->inode == ino){
//                 break;
//             }
//         i++;
//     }
//     //printf("%s has rec_len %d\n", dp->name, dp->rec_len);
//     if(dp->rec_len == BLKSIZE){
//         //how to compact if it's between two nonzero entries
//         bdalloc(pip->dev, pip->INODE.i_block[i]);
//     }
//     else if(dp->rec_len > 4*((8+dp->name_len+3)/4)){
//         //prevlen holds the length of the previous entry, and by subtracting it from cp, it moves the cp back to the previous entry
//         cp -= prevlen;
//         dp = (DIR *)cp;
//         dp->rec_len = remain + dp->rec_len;
//         put_block(pip->dev, pip->INODE.i_block[i], buf);
//     }
//     else{
//         remain = dp->rec_len;
//         //printf("dp: %s dp rec_len: %d remain: %d\n", dp->name, dp->rec_len, remain);
//         char *prevcp;
//         while(cp + dp->rec_len < buf + BLKSIZE){
//         //while((cp) < (buf + BLKSIZE)){
//             //printf("dp: %s dp rec_len: %d remain: %d\n", dp->name, dp->rec_len, remain);
//             //nextcp = cp + dp->rec_len;
//             prevcp = cp;
//             //printf("cp: %s dp: %s prevcp: %s\n", ((DIR *)cp)->name, dp->name, ((DIR *)prevcp)->name);
//             cp += dp->rec_len;
//             dp = (DIR *)cp;
//             printf("dp: %d vs cp: %d vs prevcp: %d\n", dp, cp, prevcp);
//             printf("Copying %d bytes of %s to %s\n", remain, dp->name, ((DIR *)prevcp)->name);
//             memmove(prevcp, dp, remain);
//             printf("Adding %d to %s\n", dp->rec_len, ((DIR *)cp)->name);
//         }
//         printf("dp: %d vs cp: %d vs prevcp: %d\n", dp, cp, prevcp);
//         //memmove(prevcp, dp, remain);
//         printf("dp: %s remain: %d\n", dp->name, remain);
//         dp->rec_len += remain;
//         printf("%s rec_len: %d\n", dp->name, dp->rec_len);
//         put_block(pip->dev, pip->INODE.i_block[i], buf);
//     }
//     return 1;
// }

int remove_dir(char *pathname)
{
    //printf("found rmdir\n");
    char tempDirname[64];
    char tempBasename[64];
    char tempPath[64];

    strcpy(tempPath,pathname);

    strcpy(tempBasename,basename(tempPath));

    strcpy(tempDirname,dirname(tempPath));
    printf("tempDirname: %s    tempBasename: %s    tempPath: %s\n", tempDirname, tempBasename, tempPath);

    MINODE *mip;
    MINODE *pmip;

    int pino;
    
    pino = getino(tempDirname);
    pmip = iget(dev,pino);
    printf("pmip: %d\n", pmip->ino);

    int ino = getino(tempBasename);
    mip = iget(dev, ino);

    if(S_ISDIR(mip->INODE.i_mode))
    {    
	    if(!search(pmip, tempBasename))
	        {
                return 0;
	        }
        //int ino = getino(tempBasename);
        //mip = iget(dev, ino);
        int i = 0, entries = 0;
        char buf[BLKSIZE], *cp;
        DIR *dp;
        while(i < 12){
        get_block(mip->dev, mip->INODE.i_block[i], buf);
        cp = buf;
        dp = (DIR *)cp;
        while((cp) < (buf + BLKSIZE)){
            //entries++;
            //printf("%d. next entry found: %s\n", i, dp->name);
            //printf("buf + BLKSIZE: %d  - cp: %d = %d\n", buf+BLKSIZE, cp, (buf+BLKSIZE)-cp);
            cp += dp->rec_len;
	        dp = (DIR *)cp;
            if(dp->rec_len == 0){
                break;
            }
            entries++;
        }
        i++;
        }
        printf("number of entries found: %d\n", entries);
        if(entries > 2){
            printf("That directory is not empty\n");
            return 0;
        }

        else{
        rm_child(pmip, tempBasename);

        pmip->INODE.i_links_count--;
        pmip->dirty = 1;
        iput(pmip);
        bdalloc(mip->dev, mip->INODE.i_block[0]);
        idalloc(mip->dev, mip->ino);
        iput(mip);
        return 1;
        }
    }
    return 0;
}

#endif