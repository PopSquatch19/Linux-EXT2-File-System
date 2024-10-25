/************* symlink.c file **************/

#ifndef __SYMLINK_C__
#define __SYMLINK_C__

//#include "mkdir_creat.c"
//#include "open_close.c"
//#include "type.h"

int symlink_file(char *pathname, char *pathname2)
{
    
  char tempPath[128], *tempOldFile;

  int oino;   //old ino
  int pino;  //parent
  int new_ino; 
  char buf[64];
  char tempDirname[128];
  char tempBasename[128];
  
  MINODE *omip;
  MINODE *pmip;   //parent
  MINODE *new_ip;

  //strcpy(pathTemp,pathname); //copy for creat
  // strcpy(tempOldFile,name[1]);

  
  

  //printf("pathname1 : %s pathname2: %s\n",pathname,pathname2);

  


  


  //printf("path: %s\n", pathname);
  oino = getino(pathname); //for old file
  //printf("pathname: %s\n", pathname);
  omip = iget(globalDev,oino);

  

  //printf("test1\n");

  strcpy(tempPath,pathname2);

  strcpy(tempBasename,basename(tempPath));

  strcpy(tempDirname,dirname(tempPath));

  //printf("tempBasename: %s  tempDirname: %s\n", tempBasename,tempDirname);

  pino = getino(tempDirname); //for new file
  pmip = iget(globalDev,pino);  


  //printf("pino: %d\n", pino);



  
  //printf("len of name[1]: %d\n",strlen(name[1]));

  //strcpy(name_at_1_copy,name[1]);

  //check if old_file exists and if new_file don't exist yet

  if((oino != 0) && (pino != 0))
  {

     if(S_ISDIR(pmip->INODE.i_mode))
      {    
        if(search(pmip, tempBasename))    //searches to see if basename doesnt exist
        {
          return 0;
        }
      }
      else
      {
        printf("Is not a DIR.\n");
        return -1;
      }

    //printf("test2\n");
    mycreat(pathname2);

  
    //printf("test3\n");

    new_ino = getino(pathname2); //for new file
    new_ip = iget(globalDev,new_ino); 
    
    new_ip->INODE.i_mode = 0xA1FF; 



    strcpy(tempPath,pathname);

    strcpy(tempBasename,basename(tempPath));

    strcpy(tempDirname,dirname(tempPath));

    strcpy(new_ip->INODE.i_block,tempBasename);




    new_ip->INODE.i_size = strlen(tempBasename);

  
    new_ip->dirty = 1;

    iput(new_ip);



    pmip->dirty =1;
    iput(pmip);



  }

}

int my_readlink(char *linkname, char *buf)
{
    int ino;
    MINODE *mip;


    ino = getino(linkname);
    mip = iget(dev,ino);

    if(mip->INODE.i_mode = S_IFLNK)
    {
      strcpy(buf,mip->INODE.i_block);
      return mip->INODE.i_size;
    }
    // else
    // {
    //   printf("Not a link type.");
    //   return -1;
    // }
}

#endif