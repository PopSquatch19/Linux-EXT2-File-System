/************* write_cp.c file **************/

#ifndef __WRITECP_C__
#define __WRITECP_C__

#include "read_cat.c"
#include "link_unlink.c"
//#include "type.h"

int write_file(int fd, char *buf)
{
    //printf("print b4 to_write\n");
    char to_write[256]; //maybe needs to be bigger

    //printf("inside write file before strncpy\n");

    strncpy(to_write, buf,strlen(buf));

    to_write[strlen(buf)] = 0;

    printf("to_write: %s\n", to_write);

    return my_write(fd, to_write, strlen(to_write));
}

int map(INODE inode, int lbk)   //lbk int?
{
    int blk, ibuf[256], dbl_buf[256], dbl_lbk; 

    //printf("map test1\n");
   
    // int ino;
    // ino = getino(lbk);

    //printf("map test2\n");


    if(lbk < 12) //direct blocks
    {
        //printf("map test 2.1\n");
        blk = inode.i_block[lbk]; 
    }
    else if(12 >= lbk && lbk < 256 +12) //indirect blocks
        {
            if(inode.i_block[12] == 0)
            {
                //read(dev,ibuf[256],inode.i_block[12]);      //added dev
                //get_block(dev, ibuf[256],inode.i_block[12]);
                int block_holder = balloc(dev);

                inode.i_block[12] = block_holder;
            }

            get_block(dev, inode.i_block[12], ibuf);

            blk = ibuf[lbk -12];

            if(blk == 0)
            {
                ibuf[lbk - 12] = balloc(dev);
                put_block(dev, inode.i_block[12], ibuf);
                blk = ibuf[lbk - 12];
            }
            
        }
        else //dobule indirect blocks
        {
            if(inode.i_block[13] == 0)
            {
                //mailmans alg
                dbl_lbk = lbk - 12 - 256; // should this be placed outside it as well?
                int dbl_block_holder = balloc(dev);
                inode.i_block[13] = dbl_block_holder;

            }

            get_block(dev ,inode.i_block[13], ibuf);
            blk = ibuf[dbl_lbk / 256];

            if(blk == 0)
            {
                ibuf[dbl_lbk /256] = balloc(dev);
                put_block(dev ,inode.i_block[13], ibuf);
                blk = ibuf[dbl_lbk /256];

            }

            // //read(dev,dbl_buf[256],blk);
            // get_block(dev,blk,dbl_buf);


            // if(dbl_buf[] == 0)
            // {

            //     blk = dbl_buf[dbl_lbk % 256];
            //     int extra_blk = balloc(dev);
            //     inode.i_block[12] = extra_blk;
            // }

            // //get_block(dev, inode.i_block[12],dbl_buf);
            blk = dbl_buf[dbl_lbk % 256];

            if(blk == 0)
            {
                dbl_buf[dbl_lbk % 256] = balloc(dev);
                put_block(dev, inode.i_block[12],dbl_buf);
                blk = dbl_buf[dbl_lbk % 256];
            }

    
        }
        return blk;
}

int my_write(int fd, char buf[BLKSIZE], int nbytes)     //changed buf[] to buf*
{
    int count = 0, remain, map_return;
    int blk, ino;        //?
    char wbuf[BLKSIZE];     //blksize?

    //MINODE *ip;
    MINODE *mip;

    //printf("test1\n");

    mip = oft[fd].minodeptr;

    char *cq = buf;

    //printf("test2\n");

    while(nbytes > 0)
    {
        //printf("test3\n");
        int lbk = oft->offset / BLKSIZE;
        int startByte = oft->offset % BLKSIZE;

        //printf("test4\n");

        //convert lbk to physical blk number, blk, through the i_block[] array

        map_return = map(oft[fd].minodeptr->INODE,lbk);     //wbuf? //really not sure if this right

        //printf("test5\n");


        get_block(dev,map_return,wbuf);   //read disk block into wbuf[] cahnged it from get to put ///put it back to put

        //printf("test6\n");

        char *cp = wbuf + startByte;    //cp points at startByte in wbuf[]
        remain = BLKSIZE - startByte; //# of bytes that remain in this block

        //printf("write nbytes: %d\n", nbytes);

        //printf("test7\n");

        while(remain > 0)
        {
            *cp++ = *cq++;
            oft->offset++; count++;
            
            remain --;
            nbytes--;

            if(oft->offset > mip->INODE.i_size)
            {
                mip->INODE.i_size++;
            }
            if(nbytes <= 0)
            {
                break;
            }
        }


        put_block(mip->dev,map_return,wbuf); //writing wbuf[] to disk

        //should there be more?
    }

    printf("nbytes: %d\n", nbytes);
    printf("buf after write: %s\n", buf);
    mip->dirty = 1;
    return nbytes;
}
//         int blk = oft[fd].minodeptr->INODE.i_block[lbk];
//         char kbuf[BLKSIZE];
//         get_block(dev, blk, kbuf);
//         char *cp = kbuf + start;
//         int remain = BLKSIZE - start;
//         while(remain){
//             *cp++ = *buf++;
//             oft[fd].offset++; count++;
//             remain--; nbytes--;
//             if(oft[fd].offset > oft[fd].minodeptr->INODE.i_size)
//             {
//                 oft[fd].minodeptr->INODE.i_size++;
//             }
//             if(nbytes <= 0)
//             {
//                 break;
//             }
//         }
//         put_block(dev, blk, kbuf);
//     }
//     oft[fd].minodeptr->dirty = 1;
//     return count; // Eventually: return the number of bytes written
// }

int my_cp(char *src, char *dest)
{
    int gd, n;
    char tempDirname[64];
    char tempBasename[64];
    char tempPath[64];
    char mybuf[BLKSIZE];

    // strcpy(tempPath,dest);
    

    // strcpy(tempBasename,basename(tempPath));

    // strcpy(tempDirname,dirname(tempPath));

    //printf("tempDirname: %s tempBasename: %s\n", tempDirname, tempBasename);

    fd = open_file(src,O_RDONLY);
    //gd = open_file(dest,O_RDWR | O_CREAT); //pretty sure there needs to be another value next to o_creat
    gd = open_file(dest, O_RDWR);

    //pfd();

    if(fd == -1)
    {
        printf("File might already be open or in use./n");
        return -1;
    }

    bzero(mybuf, BLKSIZE);
    
    while( n = my_read(fd, mybuf,BLKSIZE))        //BLKSIZE = 1024
    {
        if(n == 0)
        {
            break;
        }
        printf("n: %d\n", n);
        //mybuf[n] = b0;       // as a null terminating string
        my_write(gd, mybuf, n); //will probably need to change
        bzero(mybuf, BLKSIZE);
        //n++;
    }
}

#endif