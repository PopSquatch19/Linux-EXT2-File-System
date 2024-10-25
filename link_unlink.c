/************* link_unlink.c file **************/

#ifndef __LINKUNLINK_C__
#define __LINKUNLINK_C__

//#include "mkdir_creat.c"
//#include "type.h"
#include "rmdir.c"

extern int globalDev;


int mylink(char * pathname)
{
    char *old_file, *new_file;
   
    int *oino;
    char *parent, *child;
    MINODE *omip;
    MINODE *pino;  ///hmmmmm
    MINODE *pmip;  ///hmmmmm

    link_tokenize(pathname);

    //printf("First 3 items in name: %s, %s, %s\n", name[0], name[1], name[2]);

    //strncpy(old_file, name[1], strlen(name[1]));
    //strncpy(new_file, name[2], strlen(name[2]));

    //strcpy(old_file, name[1]);
    //strcpy(new_file, name[2]);

    //old_file[strlen(old_file)-1] = 0;
    //new_file[strlen(new_file)-1] = 0;

    //printf("test reach\n");

    //printf("old and new: %s %s\n", old_file, new_file);
    
   //oino = getino(old_file);
   oino = getino(name[1]);

   if(!oino){
    printf("Can't link to nonexistent file\n");
    return 0;
   }

   omip = iget(globalDev,oino);
   //printf("test1: is name[1] null = %d\n", name[1] == NULL);
   //printf("oino is %d\n", oino);
   //if((omip->INODE.i_mode != DIR_MODE) && (old_file != NULL))
   if((omip->INODE.i_mode != DIR_MODE) && (oino > 0))
   {
    printf("oino: %d\n", oino);

    
    //if(getino(new_file))
    if(getino(name[2]))
    {
        return 0;
    }

    //printf("test3\n");

    //printf("new file as it was passed in: %s\n", new_file);    

    /*parent = dirname(new_file);
    child = basename(new_file);*/
    parent = dirname(name[2]);
    child = basename(name[2]);

    pino = getino(parent);
    pmip = iget(globalDev, pino);

    //printf("new file to be entered: %s\n", child);

    enter_child(pmip,oino,child);
    //printf("test4\n");
   }

   printf("%s number of links before linking: %d\n", name[1], omip->INODE.i_links_count);
   omip->INODE.i_links_count++;
   printf("%s number of links after linking: %d\n", name[1], omip->INODE.i_links_count);
   omip->dirty = 1;
   iput(omip);
   printf("%s number of links after iput: %d\n", name[1], omip->INODE.i_links_count);
   iput(pmip);
   return 1;
}

int my_unlink(char *pathname)
{
    char *parent, child;
    int ino = getino(pathname);
    MINODE *mip = iget(dev, ino);

    if(mip->INODE.i_mode == DIR_MODE){
        return 0;
    }

    parent = dirname(pathname); child = basename(pathname);
    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);
    rm_child(pmip, pathname);
    pmip->dirty = 1;
    iput(pmip);

    printf("%s has %d links\n", pathname, mip->INODE.i_links_count);
    mip->INODE.i_links_count--;
    printf("%s has %d links now\n", pathname, mip->INODE.i_links_count);

    if(mip->INODE.i_links_count > 0){
        mip->dirty = 1;
        //printf("test1\n");
    }
    else{
        //printf("test2\n");
        for(int i = 1; i < 15; i++){
            bdalloc(dev, mip->INODE.i_block[i]);
        }
        idalloc(dev, ino);
        return 0;
        //we'll get to this
    }
    iput(mip);
    return 1;
}

int my_rm(MINODE *mip, char *pathname)
{
    return 1;
}

#endif