/************* cd_ls_pwd.c file **************/

/*#include "util.c"
  #include "type.h"*/

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";


int cd()
{
  //printf("cd: under construction READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO chdir

  int ino = getino(pathname);

  if(ino ==0)
  {
    printf("Error. Ino number value was 0.\n");
    return -1;
  }

  MINODE *mip = iget(dev,ino);

  if(!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Error. Is not DIR.\n");
    return -1;
  }

  iput(running->cwd);

  running->cwd= mip;

}

int ls_file(MINODE *mip, char *name)
{
  //printf("ls_file: to be done: READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
  char buf[64];


int r, i;
char ftime[64];

//printf("ls_file test 1\n");


//mip->INODE.

if ((mip->INODE.i_mode & 0xF000) == 0x8000) // if (S_ISREG())
printf("%c",'-');
if ((mip->INODE.i_mode & 0xF000) == 0x4000) // if (S_ISDIR())
printf("%c",'d');
if ((mip->INODE.i_mode & 0xF000) == 0xA000) // if (S_ISLNK())
printf("%c",'l');
for (i=8; i >= 0; i--){
if (mip->INODE.i_mode & (1 << i)) // print r|w|x
printf("%c", t1[i]);
else
printf("%c", t2[i]); // or print -
}
printf("%4d ",mip->INODE.i_links_count); // link count
printf("%4d ",mip->INODE.i_gid); // gid
printf("%4d ",mip->INODE.i_uid); // uid
printf("%8d ",mip->INODE.i_size); // file size
// print time

//printf("ls_file test 2\n");

 time_t t = mip->INODE.i_ctime;
strcpy(ftime, ctime(&t)); // print time in calendar form

//printf("ls_file test 2.1\n");
ftime[strlen(ftime)-1] = 0; // kill \n at end
printf("%s ",ftime);

//printf("ls_file test 2.2\n");
// print name
printf("%s", basename(name)); // print file basename
//printf("\n");
// print -> linkname if symbolic file

//printf("ls_file test 2.3\n");

//printf("ls_file test 3\n");
if ((mip->INODE.i_mode & 0xF000)== 0xA000)
{
  // use readlink() to read linkname
  
  my_readlink(name,buf);        ////EDITED 11/29/22
  printf(" -> %s", buf);
//ssize_t result = readlink(fname,linkname,MAX-1);
// if(result != -1)
// {
//   linkname[result] = 0;
//   printf(" -> %s", linkname); // print linked name

// }

}
printf("\n"); 

}

int ls_dir(MINODE *mip)
{
  //printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  //printf("ls_dir test 1\n");

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;

  //printf("ls_dir test 2\n");
  
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;

     //printf("ls_dir test 3\n");
	
     //printf("%s  ", temp);

     MINODE *tmip = iget(dev,dp->inode);

     //printf("ls_dir test 4\n");

     //printf("%s rec_len: %d\n", dp->name, dp->rec_len);

     ls_file(tmip,temp);

     /*if(tmip->INODE.i_ctime == 0){
      printf("buf + BLKSIZE: %d  - cp: %d = %d\n", buf+BLKSIZE, cp, (buf+BLKSIZE)-cp);
      break;
     }*/

     cp += dp->rec_len;
     dp = (DIR *)cp;

     //printf("ls_dir test 5\n");

  }
  printf("\n");
}

int ls(char *pathname)
{
  //printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");

  MINODE *m;

  int n =getino(pathname); //return ino #
  m = iget(dev,n);  //etunrs entire minode pointer

 //printf("test1\n");

  if(!strcmp(pathname,""))
  {
    // printf("test2\n");
    ls_dir(running->cwd);
    
    
    
    //printf("test3\n");
  }
  else
    {
      ls_dir(m);
    }

  //printf("test4\n");


  // ls_dir(running->cwd);
}

char *rpwd(MINODE *wd)
{
  //printf("pwd: READ HOW TO pwd in textbook!!!!\n");
  if (wd == root){
    //printf("/");
    return 0;
  }
  u32 my_ino, parent_ino;
  char *myname = malloc(32);
  parent_ino = findino(wd, &my_ino); //find ino gives my_ino the ino of . , and returns the ino of ..
  MINODE *pip = iget(dev, parent_ino);
  findmyname(pip, my_ino, myname);
  //printf("myname is %s\n", myname);
  rpwd(pip);
  printf("/%s", myname);
}

char *pwd(){
  printf("running: %s\n", ((DIR *)running->cwd)->name);
  if(running->cwd == root){
    printf("/");
  }
  else{
    rpwd(running->cwd);
  }
  printf("\n");
  return 0;
}