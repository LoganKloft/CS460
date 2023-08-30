int search(INODE* ip, char* file)
{
  uprintf("===========SEARCH============\n");
  uprintf("TARGET: %s\n", file);
  DIR *dp;
  char buf1[BLKSIZE], name[256], *cp;

  getblk(ip->i_block[0], buf1);
  dp = (DIR *)buf1;
  cp = buf1;

  while (cp < buf1 + BLKSIZE)
  {
    strncpy(name, dp->name, dp->name_len);
    uprintf("-file-> %s %d\n", name, dp->inode);

    if (strcmp(name, file) == 0)
    {
      uprintf("FOUND: %s\n", file);
      uprintf("===========SUCCESS============\n");
      return dp->inode;
    }

    cp += dp->rec_len;
    dp = (DIR *)cp;

    if (dp->rec_len == 0) return 0;
  }

  uprintf("==========FAIL============\n");
  return 0;
}

int boot()
{ 
  char   *name[2];  // name[0]="boot";   name[1]="eos";
  char   *address = 0x100000;  // EOS kernel loading address     
  uprintf("> starting booter\n");

  // 1. GD is in block #2, get the inodes start block from GD
  GD *gp;
  INODE *ip;
  char buf1[BLKSIZE], buf2[BLKSIZE], buf3[BLKSIZE];

  getblk(2, buf1);
  gp = (GD *)buf1;
  int iblk = gp->bg_inode_table;

  // 2. Get the root INODE which is the second INODE
  getblk(iblk, buf2);
  ip = (INODE *)buf2 + 1;

  // 3. search for 'boot' in '/'
  int ino = search(ip, "boot");
  int block = (ino - 1) / 8 + iblk;
  int offset = (ino - 1) % 8;
  getblk(block, buf3);
  ip = (INODE *)buf3 + offset;

  // 4. search for 'eos' in 'boot'
  ino = search(ip, "eos");
  block = (ino - 1) / 8 + iblk;
  offset = (ino - 1) % 8;
  getblk(block, buf3);
  ip = (INODE *)buf3 + offset;

  // 5. load 'eos' block data to 1MB offset
  for (int i = 0; i < 12; i++)
  {
    if (ip->i_block[i] != 0) {
      getblk(ip->i_block[i], address);
      address += 1024;
    }
  }
  u32 ubuf[256], *ubufp;
  getblk(ip->i_block[12], ubuf);
  ubufp = (u32 *)ubuf;
  while(*ubufp)
  {
    getblk(*ubufp, address);
    address += 1024;
    ubufp++;
  }

  uprintf("> exiting booter\n");

  // 6. return control to go()
  return 1;
}