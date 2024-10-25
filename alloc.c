//#include <stdlib.h>
//#include "util.c"

char buf[1024];
SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;   
//possible redef errors


int tst_bit(char *buf, int bit)
{
    return buf[bit/8] & (1 << (bit % 8));

}

int set_bit(char *buf, int bit)
{
    buf[bit/8] |= (1 << (bit % 8));   
}

int clr_bit(char * buf, int bit){
  buf[bit/8] &= ~(1 << (bit%8));
}

int decFreeInodes(int dev)
{
  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int decFreeBlocks(int dev)
{
  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  //MTABLE *mp = (MTABLE *)get_mtable(dev);
  //printf("insidetest2 imap: %d\n", mp->imap);
  //printf("insidetest2\n");
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){ // use ninodes from SUPER block
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
	      put_block(dev, imap, buf);

	      decFreeInodes(dev);

	      printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}

int balloc(int dev) //allocates a free disk block (number) from a device
{
    int  i;
    char buf[BLKSIZE];
    //MTABLE *mp = (MTABLE *)get_mtable(dev);

    // read inode_bitmap block
    get_block(dev, bmap, buf);

    for (i=0; i < nblocks; i++)
    {
        if (tst_bit(buf, i)==0)
        {
            set_bit(buf, i);
	          put_block(dev, bmap, buf);

	          decFreeBlocks(dev);

	          //printf("allocated disk block = %d\n", i+1); // bits count from 0; ino from 1
	          return (i+1);
        }
    }
    return 0;
}

int incFreeInodes(int dev){
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 1, buf);
}

int idalloc(int dev, int ino){
  int i;
  char buf[BLKSIZE];
  MTABLE *mp = (MTABLE *)get_mtable(dev);
  /*if(ino > mp->ninodes){
    printf("inumber %d out of range\n", ino);
    return 0;
  }*/

  //get inode bitmap
  get_block(dev,mp->imap, buf);
  clr_bit(buf, ino-1);

  //write buf back
  put_block(dev, mp->imap, buf);

  //update free inode count in SUPER and GD
  incFreeInodes(dev);
}

int incFreeBlocks(int dev){
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 1, buf);
}

int bdalloc(int dev, int bno){
  int i;
  char buf[BLKSIZE];
  //printf("test1\n");
  MTABLE *mp = (MTABLE *)get_mtable(dev);
  /*if(bno > mp->nblocks){
    printf("bnumber %d out of range\n", bno);
  }*/

  //get block map
  //printf("test3\n");
  get_block(dev, mp->bmap, buf);
  clr_bit(buf, bno-1);

  //printf("test4\n");
  //write buf back
  put_block(dev, mp->bmap, buf);

  //update free blocks count in SUPER and GD
  incFreeBlocks(dev);
}