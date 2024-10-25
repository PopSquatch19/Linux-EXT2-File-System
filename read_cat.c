/************* read_cat.c file **************/

#ifndef __READCAT_C__
#define __READCAT_C__

#include "open_close.c"
//#include "write_cp.c"
//#include "util.c" //added due to get_block not letting me see parameter types but include might not need to have

int read_file(int fd, char *buf,int nbytes)
{
    //char buf[BLKSIZE];
    int mode;
    
    mode = proc->fd[fd]->mode;

  if(mode == 0 || mode == 2)    //0 = read, 2 = read/write
  {
    return my_read(fd,buf,nbytes);
  }
}

int my_read(int fd, char buf[BLKSIZE], int nbytes)
{
    int avil, blk, lbk, remain, startByte, map_return, count = 0, offset = oft[fd].offset; //number of bites available, number of bites read, offset
    char wbuf[BLKSIZE];

    avil = oft[fd].minodeptr->INODE.i_size - offset; 

    char *cq = buf;

    while(nbytes && avil)
    {
        lbk = oft->offset / BLKSIZE;
        startByte = oft->offset % BLKSIZE;
        //lbk = offset / BLKSIZE;
        //startByte = offset % BLKSIZE;
        // int lbk = oft[fd].minodeptr->INODE.i_size/BLKSIZE; //logical block
        // int start = offset % BLKSIZE; //start byte

        //printf("offset: %d lbk: %d\n", offset, lbk);


        //blk = oft[fd].minodeptr->INODE.i_block[lbk]; //convert logical block lbk to physical block number blk using INODE.i_block[] . It's probably wrong

        map_return = map(oft[fd].minodeptr->INODE,lbk);     //wbuf? //really not sure if this right

        //printf("map_return: %d\n", map_return);

        //printf("offset: %d startByte: %d\n", offset, startByte);
      
        get_block(dev,map_return,wbuf);   //read disk block into wbuf[]

        //puts(wbuf);


        char *cp = wbuf + startByte;    //cp points at startByte in wbuf[]
        //char *cp = wbuf +;    //cp points at startByte in wbuf[]
      //  printf("Startbyte value: %d\n", startByte);
        remain = BLKSIZE - startByte; //# of bytes that remain in this block

        //strcpy(extra_buf,buf);

        while(remain){
         //   printf("Enter while remain loop\n");
            //printf("buf: %s\n", buf);   //cant even print this line. without it, it stops when line 56 is ran
            //printf("cp: %s\n", cp);
            *cq++ = *cp++;   //this line is causing the seg fault and I'm not sure why. 
         //   printf("Here\n");
            /* ^^
            buf (extra buf) doesn't acutally have anything it at the moment so it can't increment of course. 
            I am not sure why it seg faults here. My guess is that it

            */
         //   printf("remain: %d avil: %d nbytes: %d\n", remain, avil, nbytes);
            oft->offset++; count++;
            remain--; avil--; nbytes--;
            if(nbytes <= 0 || avil <= 0)
            {
                break;
            }
        }
    }
    //printf("buf: %s\n", buf);
    return count; // Eventually: Return the actual number of bytes read
}

int cat_file(char *pathname)
{
    char mybuf[BLKSIZE], dummy = 0;
    //char* mybuf = malloc(BLKSIZE);
    int n;

    int fd = open_file(pathname, 0);

    if(fd == -1){
        printf("Cannot cat file\n");
        return -1;
    }

    bzero(mybuf, BLKSIZE);
    
    while( n = my_read(fd, mybuf,BLKSIZE))        //BLKSIZE = 1024
    {
        //mybuf[n] = b0;       // as a null terminating string
        printf("%s", mybuf); //will probably need to change
        bzero(mybuf, BLKSIZE);
        //n++;
    }
    //printf("%s\n", mybuf); //cat out the contents of pathname
    // while( n = my_read(fd, mybuf,BLKSIZE))        //BLKSIZE = 1024
    // {
    //     mybuf[n] = 0;       // as a null terminating string
    //     printf("%d. %s\n", n, mybuf); //will probably need to change
    //     //n++;
    // }
    printf("\n");
    //printf("End of cat\n");
    my_close(fd);
    return 0;
}

#endif