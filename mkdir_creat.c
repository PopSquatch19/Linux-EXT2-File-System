#include <stdlib.h>
#include <libgen.h>

//#include "type.h"
//#include "util.c"
#include "alloc.c"

char enter_child(MINODE *pip, int ino, char *basename)
{
  char buf[BLKSIZE], *cp;
  for(int i = 0; i < 12; i++){
    if(pip->INODE.i_block[i] != 0){
      get_block(pip->dev, pip->INODE.i_block[i], buf);
      dp = (DIR *)buf;
      cp = buf;
      while((cp + dp->rec_len) < (buf + BLKSIZE)){
	cp += dp->rec_len;
	dp = (DIR *)cp;
      }
      int remain = dp->rec_len - 4*((8+dp->name_len+3)/4);
      if(remain >= 4*((8+strlen(basename)+3)/4)){
	dp->rec_len = 4*((8+dp->name_len+3)/4);
	/*char *newent[BLKSIZE];
	  dp = (DIR *)newent;*/
	char *cp = dp;
	cp += dp->rec_len;
	dp = (DIR *)cp;
	dp->inode = ino;
	dp->rec_len = remain;
	dp->name_len = strlen(basename);
	strcpy(dp->name, basename);
	put_block(pip->dev, pip->INODE.i_block[i], buf);
	return 1;
      }
    }
    else{
      char *buf[BLKSIZE];
      get_block(pip->dev, pip->INODE.i_block[i], buf);
      dp->inode = ino;
      dp->rec_len = BLKSIZE;
      dp->name_len = strlen(basename);
      strcpy(dp->name, basename);
      //buf = dp;
      //iput(iget(pip->dev, dp->inode));
      put_block(pip->dev, pip->INODE.i_block[i], buf);
      return 1;
    }
  }
}


int k_mkdir(MINODE *pmip, char *baseName)
{
    int ino = ialloc(dev);
    int blk = balloc(dev); //blksize?

    char tempMode[16];
    
    MINODE *mip;

    mip = iget(dev, ino);
    mip->INODE.i_mode = 0x41ED;
    mip->INODE.i_block[0] = blk; // other i_block [] =0???
    for(int i = 1; i < 15; i++){

     mip->INODE.i_block[i] = 0;
    }
    time_t t = time(0);

    
    mip->INODE.i_uid = pmip->INODE.i_uid;
    mip->INODE.i_size = BLKSIZE;
    mip->INODE.i_atime = t;
    mip->INODE.i_ctime = t;
    mip->INODE.i_mtime = t;
    mip->INODE.i_gid = pmip->INODE.i_gid;
    mip->INODE.i_links_count = 2;
    mip->dirty = 1;

    iput(mip); // write INODE back to disk

    // mip->INODE.i_block[0] = basename(baseName); //probably wrong

    char *buf[BLKSIZE];
    DIR *dp = (DIR *)buf;
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';
    dp = (char *)dp+12;
    dp->inode = ino;
    dp->rec_len = BLKSIZE-12;
    dp->name_len = 2;
    dp->name[0] = dp->name[1] = '.';

    put_block(mip->dev, blk, buf); // write back to disk

    enter_child(pmip, ino, baseName);

   
}

int mymkdir(char *pathname)
{

    char tempDirname[64];
    char tempBasename[64];
    char tempPath[64];

    strcpy(tempPath,pathname);

    strcpy(tempBasename,basename(tempPath));

    strcpy(tempDirname,dirname(tempPath));
    printf("tempDirname: %s    tempBasename: %s    tempPath: %s\n", tempDirname, tempBasename, tempPath);

    MINODE *mip;
    MINODE *pmip;

    int pino; //or char *pino?
    
    pino = getino(tempDirname); //dirname?
    pmip = iget(dev,pino);
    printf("pmip: %d\n", pmip->ino);

    if(S_ISDIR(pmip->INODE.i_mode))
      {    
	 if(search(pmip, tempBasename))
	   {
	     return 0;
	   }
      

    k_mkdir(pmip, tempBasename);

    pmip->INODE.i_links_count++;  //added 12/2/22
    pmip->dirty = 1;
    iput(pmip);
      }
    else{
      printf("That is not a directory\n");
      return 0;
    }
}



int kcreat(MINODE *pmip, char *name)
{

    int ino = ialloc(dev);
    int blk = balloc(dev); //blksize?

   
    
    MINODE *mip;

    mip = iget(dev, ino);
    mip->INODE.i_mode = 0x81A4;
    mip->INODE.i_block[0] = blk; // other i_block [] =0???
    for(int i = 1; i < 15; i++)
    {
     mip->INODE.i_block[i] = 0;
    }
    //mip =  mip->dirty;

    //time_t t = time(0);
    time_t t = time(0);
    //printf("time: %ld\n", t);
    //strcpy(ftime, ctime(&t)); // print time in calendar form  EDITTTTTTTTTTT

    
    mip->INODE.i_uid = pmip->INODE.i_uid;
    mip->INODE.i_size = 0;
    mip->INODE.i_atime = t;
    mip->INODE.i_ctime = t;
    mip->INODE.i_mtime = t;
    mip->INODE.i_gid = pmip->INODE.i_gid;
    mip->INODE.i_links_count = 1;
    mip->dirty = 1;

    iput(mip); // write INODE back to disk

    put_block(mip->dev, blk, buf); // write back to disk

    enter_child(pmip,ino,name);

}




int mycreat(char *pathname)
{
    char tempDirname[64];
    char tempBasename[64];
    char tempPath[64];

    strcpy(tempPath,pathname);

    strcpy(tempBasename,basename(tempPath));

    strcpy(tempDirname,dirname(tempPath));
    printf("tempDirname: %s    tempBasename: %s    tempPath: %s\n", tempDirname, tempBasename, tempPath);

    MINODE *mip;
    MINODE *pmip;

    int pino; //or char *pino?
    
    pino = getino(tempDirname); //dirname?
    pmip = iget(dev,pino);
    printf("pmip: %d\n", pmip->ino);

    if(S_ISDIR(pmip->INODE.i_mode))
      {    
	 if(search(pmip, tempBasename))
	   {
	     return 0;
	   }
      

    //kmkdir(pmip, tempBasename);
   kcreat(pmip,tempBasename);

   pmip->dirty = 1;
   iput(pmip);
      }
      else{
        printf("That is not a dierctory\n");
        return 0;
      }
}
