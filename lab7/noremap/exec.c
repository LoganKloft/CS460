#define UPN 1
#define UVA UPN*0x100000

int exec(char *uline)
{
  int i, *ip;
  char *cp, kline[64]; 
  PROC *p = running;
  char file[32], filename[32];
  int *usp, *ustacktop;
  u32 BA, Btop, Busp, uLINE;
  char line[32];

  printf("EXEC: proc %d uline=%x\n", running->pid, uline);

  // line is in Umode image at p->pgdir[UPN]&0xFFF00000=>can access from Kmode
  // char *uimage = (char *)(p->pgdir[UPN] & 0xFFF00000);
  BA = (p->pgdir[UPN] & 0xFFFF0000);
  Btop = BA + 0x100000;  // top of 1MB Uimage
  printf("EXEC: proc %d Uimage at %x uline=%x\n", running->pid, BA, uline);
 
  //uLINE = BA + ((int)uline);
  uLINE = (int)uline;
  kstrcpy(kline, (char *)uLINE);
  // NOTE: may use uline directly 

  printf("EXEC: proc %d line = %s\n", running->pid, kline); 

  // first token of kline = filename
  cp = kline; i=0;
  while(*cp != ' '){
    filename[i] = *cp;
    i++; cp++;
  } 
  filename[i] = 0;
  /*
  kstrcpy(file, "/bin/");
  kstrcat(file, filename);
  */
  BA = p->pgdir[UPN] & 0xFFFF0000;
  kprintf("load file %s to %x\n", file, BA);

  // load filename to Umode image 
  if (load(filename, p) <= 0)
     return -1;

  // UPN=1 defined in type.h. It determines the Umode page table entry number

  BA = p->pgdir[UPN] & 0xFFFF0000;
  Btop = BA + 0x100000;
  Busp = Btop - 128 + 0x80000000;
  printf("BA=%x Busp=%x\n", BA, Busp);

  cp = (char *)Busp;
  strcpy(cp, kline);
  printf("stack contents=%s UVA=%x\n", cp, 0);

  // KCW: for 1MB Umode image, ustack top is at VA=2MB, set it to 2MB-128
  cp = (char *)(0x200000 - 128);

  p->kstack[SSIZE-14] = p->kstack[SSIZE-13] = (int)(cp); 
  p->usp = (int *)(cp);

  p->kstack[SSIZE-1] = (int)(UVA);

  p->kstack[SSIZE-14] = p->kstack[SSIZE-13] = (int)(cp); 
  p->kstack[SSIZE-1] = (int)(UVA);
 
  kprintf("kexec exit\n");
  return 0;
}
