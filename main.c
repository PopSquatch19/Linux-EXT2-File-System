/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <ext2fs/ext2_fs.h>

#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;


extern MINODE *iget();


char *rootdev = "disk2";


MINODE minode[NMINODE]; // In memory inodes
MINODE *root; // Root minode
MTABLE mtable[NMTABLE]; // Mount tables

OFT oft[NOFT]; // Opened file instances
//PROC proc[NPROC]; // Process structures
//PROC *running; //Currently running (executing) process


int globalDev;

// char *name[64]; // token string pointers
// char gline[256]; // holds token strings, each pointed to by a name[i]

int nname; // number of token strings
int inode_start; // Inode start block



MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[64];  // assume at most 64 components in pathname
int   n;         // number of component strings

int  fd, dev;
int  nblocks, ninodes, bmap, imap, iblk, iblock;
char line[128], line2[128], cmd[32], pathname[128], pathname2[128];

#include "cd_ls_pwd.c"
#include "symlink.c"
//#include "link_unlink.c"
#include "write_cp.c"
//#include "mount_unmount.c"
//#include "rmdir.c"

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i+1;           // pid = 1, 2
    p->uid = p->gid = 0;    // uid = 0: SUPER user
    p->cwd = 0;             // CWD of process
  }
  for (i=0; i<NMTABLE; i++){
    mtable[i].dev = 0;
  }
  for (i=0; i<64; i++){
    oft[i].refCount = 0;
  }
  for (i=0; i<NPROC; i++){
    proc[i].status = READY;
    proc[i].pid = i;
    proc[i].uid = i;
    for (j=0; j<NFD; j++){
      proc[i].fd[j] = 0;
    }
    proc[i].next = &proc[i+1];
  }
  proc[NPROC-1].next = &proc[0];
  running = &proc[0];
}

// load root INODE and set root pointer to it
int mount_root(char *rootDev)
{  
  //printf("mount_root()\n");
  //root = iget(dev, 2);
  int i;
  MTABLE *mp;
  SUPER *sp;
  GD *gp;
  char buf[BLKSIZE];

  globalDev = open(rootdev, O_RDWR);

  if (globalDev < 0){
    printf("panic : can’t open root device\n");
    exit(1);
  }

  /* get super block of rootdev */
  get_block(globalDev, 1, buf);
  sp = (SUPER *)buf;

  /* check magic number */
  if (sp->s_magic != SUPER_MAGIC){
    printf("super magic=%x : %s is not an EXT2 filesys\n",
    sp->s_magic, rootdev);
    exit(0);
  }

  // fill mount table mtable[0] with rootdev information
  mp = &mtable[0]; // use mtable[0]
  mp->dev = globalDev;

  // copy super block info into mtable[0]
  ninodes = mp->ninodes = sp->s_inodes_count;
  nblocks = mp->nblocks = sp->s_blocks_count;
  strcpy(mp->devName, rootdev);
  strcpy(mp->mntName, "/");
  get_block(globalDev, 2, buf);
  gp = (GD *)buf;
  bmap = mp->bmap = gp->bg_block_bitmap;
  imap = mp->imap = gp->bg_inode_bitmap;
  iblock = mp->iblock = gp->bg_inode_table;
  printf("bmap=%d imap=%d iblock=%d\n", bmap, imap, iblock);

  // call iget(), which inc minode’s refCount
  
  root = iget(globalDev, 2); // get root inode
  mp->mntDirPtr = root; // double link
  root->mptr = mp;

  // set proc CWDs
  for (i=0; i<NPROC; i++) // set proc’s CWD
    proc[i].cwd = iget(globalDev, 2); // each inc refCount by 1

  printf("mount : %s mounted on / \n", rootdev);
  return 0;
}

int quit()
{
  int i;

  for (i=0; i<NMINODE; i++)
  {
    MINODE *mip = &minode[i];
    if (mip->refCount && mip->dirty)
    {
      mip->refCount = 1;
      iput(mip);
    }
  }

  exit(0);
}

char *disk = "disk2";     // change this to YOUR virtual

int main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){    
    printf("open %s failed\n", disk);
    exit(1);
  }

  dev = fd;    // global dev same as this fd   

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

  init();  

 //mount_root(rootdev);
 
  root = iget(dev,2);

  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  // // WRTIE code here to create P1 as a USER process
  
  while(1){
    printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|symlink|unlink|open|close|write|read|cp|cat|quit] ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;

    strcpy(line2,line);
    //line2[128] = 0;

    sscanf(line, "%s %s", cmd, pathname);
    printf("cmd=%s pathname=%s pathname2=%s\n", cmd, pathname, pathname2);
  
    if (strcmp(cmd, "ls")==0)
       ls(pathname);
    else if (strcmp(cmd, "cd")==0)
       cd();
    else if (strcmp(cmd, "pwd")==0)
      //pwd(running->cwd);
       pwd();
    else if(strcmp(cmd, "mkdir")==0)
       mymkdir(pathname);
    else if(strcmp(cmd, "creat")==0)
       mycreat(pathname);
    else if(strcmp(cmd, "rmdir")==0)
        remove_dir(pathname);
    else if(strcmp(cmd, "link")==0)
       mylink(line);
    else if(strcmp(cmd, "symlink")==0)
      {
        sscanf(line, "%s %s %s", cmd, pathname, pathname2);
        symlink_file(pathname,pathname2);
      }
    else if(strcmp(cmd, "unlink")==0)
        my_unlink(pathname);
    else if(strcmp(cmd, "open") == 0)
        {
        sscanf(line, "%s %s %s", cmd, pathname, pathname2);
        open_file(pathname, atoi(pathname2));
        }
    else if(strcmp(cmd, "close") == 0)
        my_close(atoi(pathname));
    else if(strcmp(cmd, "write") == 0)
    {
      sscanf(line, "%s %s %s", cmd, pathname, pathname2);
      write_file((atoi(pathname)),pathname2); //pathname = fd , pathname2 = buf <- which is what we write
    }
    // else if(strcmp(cmd, "read") == 0)
    // {
    //   sscanf(line, "%s %s %s", cmd, pathname, pathname2);
    //   read_file((atoi(pathname)),atoi(pathname2)); //pathname = fd , pathname2 = nbytes 
    // }   
    else if(strcmp(cmd, "cp") == 0)
    {
        sscanf(line, "%s %s %s", cmd, pathname, pathname2);
        my_cp(pathname, pathname2);
    }
    else if(strcmp(cmd, "cat")==0)
        cat_file(pathname);
    else if(strcmp(cmd, "pfd") == 0)
        pfd();
    else if (strcmp(cmd, "quit")==0)
       quit();
  }
}


