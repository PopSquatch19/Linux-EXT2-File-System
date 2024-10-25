/************* open_close_lseek.c file **************/

#ifndef __OPENCLOSELSEEK_C__
#define __OPENCLOSELSEEK_C__

#include "mkdir_creat.c"

int truncate(MINODE *mip)
{
  return 1;
}

int open_file(char *pathname, int mode)
{
  //get ino, allocate one if need be
  int oftent, ino;

  ino  = getino(basename(pathname));

  if(ino == 0)
  {
    mycreat(pathname);
    ino = getino(basename(pathname));
  }

  //get minode
  MINODE *mip;
  mip = iget(dev, ino);

  //check to see if the file is open for something other than read
  for(int i = 0; i < NFD; i++){
    if(proc[running->pid].fd[i] != 0){
      if(proc[running->pid].fd[i]->minodeptr == mip){
        if(proc[running->pid].fd[i]->mode != 0){
          printf("That file is already open for an incompatible mode\n");
          return -1;
        }
      }
    }
  }

  //allocate oft for file
  for(int i = 0; i < 64; i++)
  {
    if(oft[i].refCount == 0)
    {
      oft[i].mode = mode;
      oft[i].refCount = 1;
      oft[i].minodeptr = mip;
      
      if(mode == 3)
      {
        oft[i].offset = mip->INODE.i_size;
      }
      else if(mode == 0 || mode == 1 || mode == 2)
      {
        oft[i].offset = 0;
      }
      else
      {
        printf("Illegal file mode\n");
        return -1;
      }
      oftent = i;
      break;
    }
  }

    //find open fd in earliest proc
    //for(int i = 0; i < NPROC; i++){
      for(int i = 0; i < NFD; i++){
        //if(proc[i].fd[j] != 0){
        if(proc[running->pid].fd[i] == 0){
          proc[running->pid].fd[i] = &oft[oftent];
          //return oftent;
          return i;
        }
      //}
    //}
  } // Eventually: return file descriptor of opened file
}

int my_close(int fd)
{
  if(oft[fd].refCount == 0){
    printf("That is not an opened file\n");
    return 0;
  }

  if(proc[running->pid].fd[fd] != 0){
    oft[fd].refCount--;
    if(oft[fd].refCount == 0){
      iput(oft[fd].minodeptr);
    }
  }

  proc[running->pid].fd[fd] = 0;

  return 1;
}

int close_file(int fd)
{
  return 0;
}

int my_lseek(int fd, int position)
{
  int ogpos = oft[fd].offset;
  if(position >= 0 && position < oft[fd].minodeptr->INODE.i_size){
    oft[fd].offset = position;
    return ogpos; // Eventually: return original position in file
  }

  return 0;
}

int pfd()
{
  printf("FD  Mode     Offset  RefCount  INODE\n");
  for(int i = 0; i < NFD; i++){
    if(proc[running->pid].fd[i] != 0){
      if(proc[running->pid].fd[i]->mode == 0)
        printf("%d   read %8d       %d     [%d, %d]\n", i, proc[running->pid].fd[i]->offset, proc[running->pid].fd[i]->refCount, proc[running->pid].fd[i]->minodeptr->dev, proc[running->pid].fd[i]->minodeptr->ino);
      if(proc[running->pid].fd[i]->mode == 1)
        printf("%d   write %7d       %d     [%d, %d]\n", i, proc[running->pid].fd[i]->offset, proc[running->pid].fd[i]->refCount, proc[running->pid].fd[i]->minodeptr->dev, proc[running->pid].fd[i]->minodeptr->ino);
      if(proc[running->pid].fd[i]->mode == 2)
        printf("%d read/write %4d       %d     [%d, %d]\n", i, proc[running->pid].fd[i]->offset, proc[running->pid].fd[i]->refCount, proc[running->pid].fd[i]->minodeptr->dev, proc[running->pid].fd[i]->minodeptr->ino);
      if(proc[running->pid].fd[i]->mode == 3)
        printf("%d  append %7d       %d     [%d, %d]\n", i, proc[running->pid].fd[i]->offset, proc[running->pid].fd[i]->refCount, proc[running->pid].fd[i]->minodeptr->dev, proc[running->pid].fd[i]->minodeptr->ino);
    }
  }
  return 1;
}

int dup(int fd)
{
    return 1;
}

int dup2(int fd, int gd)
{
    return 1;
}

#endif