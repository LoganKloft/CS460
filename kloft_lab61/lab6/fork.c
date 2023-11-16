
int body(), goUmode();

/*************** kfork(filename)***************************
kfork() a new proc p with filename as its UMODE image.
Same as kfork() before EXCEPT:
1. Each proc has a level-1 pgtable at 6MB, 6MB+16KB, , etc. by pid
2. The LOW 258 entries of pgtable ID map 258 MB VA to 258 MB PA  
3. Each proc's UMODE image size = 1MB at 8MB, 9MB,... by pid=1,2,3,..
4. load(filenmae, p); load filenmae (/bin/u1 or /bin/u2) to p's UMODE address
5. set p's Kmode stack for it to 
           resume to goUmode
   which causes p to return to Umode to execcute filename
***********************************************************/

int kfork(char *filename)
{
  int i; 
  int pentry, *ptable;

  PROC *p = dequeue(&freeList);
  if (p==0){
    kprintf("kfork failed\n");
    return (PROC *)0;
  }

  printf("kfork %s\n", filename);
  
  p->ppid = running->pid;
  p->parent = running;
  p->status = READY;
  p->priority = 1;
  p->child = p->sibling = 0;

  // insert into parent
  if (p->parent->child == 0)
  {
    p->parent->child = p;
  }
  else
  {
    PROC* cur = p->parent->child;
    while (cur->sibling)
    {
      cur = cur->sibling;
    }
    cur->sibling = p;
  }

  // build p's pgtable 
  p->pgdir = (int *)(0x600000 + (p->pid - 1)*0x4000);
  ptable = p->pgdir;

  // initialize pgtable
  for (i=0; i<4096; i++)
    ptable[i] = 0;

  pentry = 0 | 0x412;
  for (i=0; i<258; i++){
    ptable[i] = pentry;
    pentry += 0x100000;
  }

  // UMODE VA mapping: Assume each proc has a 1MB Umode space at 8MB+(pid-1)*1MB
  // ptable entry flag=|AP=11|0|dom1|1|CB10|=110|0001|1|1110|=0xC3E or 0xC32    

  ptable[2048] = (0x800000 + (p->pid - 1)*0x100000)|0xC32;   // CB=00
  ptable[2049] = (0x1000000 + (p->pid - 1)*0x100000)|0xC32;
  ptable[2050] = (0x1800000 + (p->pid - 1)*0x100000)|0xC32;
  ptable[2051] = (0x2000000 + (p->pid - 1)*0x100000)|0xC32;
  // entries 2052 to 4095 all 0 for INVALID

  // load filename to Umode image area at 7MB+(pid-1)*1MB
  load(filename, p); // p->PROC containing pid, pgdir, etc
  
  // kstack must contain a syscall frame FOLLOWed by a resume frame
  for (i=1; i<29; i++)  // all 28 cells = 0
     p->kstack[SSIZE-i] = 0;

  // p->kstack[SSIZE-1] = (int)0x80000000;
  p->kstack[SSIZE-1] = VA(0);

  p->kstack[SSIZE-15] = (int)goUmode;

  p->ksp = &(p->kstack[SSIZE-28]);

  // p->usp = (int *)(0x80400000);
  p->usp = (int *)VA(0x400000);

  // very easy to follow code
  switchPgdir((u32)p->pgdir);
  char *name = "My name is Logan Kloft";
  int namelen = strlen(name);
  p->usp -= namelen;
  int* start = p->usp + 1;
  char* cp = start;
  for(i = 0; i <= namelen; i++)
  {
    *cp = name[i];
    cp++;
  }
  *cp = 0;
  p->kstack[SSIZE - 14] = start;
  switchPgdir((u32)running->pgdir);
  
  p->cpsr = (int *)0x10;    // saved cpsr = previous mode was Umode

  enqueue(&readyQueue, p);

  kprintf("proc %d kforked a child %d: ", running->pid, p->pid); 
  kprintf("USP: %x\n", p->usp);
  printQ(readyQueue);

  return p->pid;
}

// works if set p->pgdir = running->pgdir
// runs if set kstack[SSIZE - 1] = VA(0) (but this is like exec u1)
int fork()
{
  int i;
  int pentry, *ptable;
  char *PA, *CA;

  PROC *p = dequeue(&freeList);
  if (p==0)
  { 
    printf("fork failed\n"); return -1; 
  }

  p->ppid = running->pid;
  p->parent = running;
  p->status = READY;
  p->priority = 1;
  p->child = p->sibling = 0;

  // insert into parent
  if (p->parent->child == 0)
  {
    p->parent->child = p;
  }
  else
  {
    PROC* cur = p->parent->child;
    while (cur->sibling)
    {
      cur = cur->sibling;
    }
    cur->sibling = p;
  }

  // build p's pgtable 
  p->pgdir = (int *)(0x600000 + (p->pid - 1)*0x4000);
  ptable = p->pgdir;

  // initialize pgtable
  for (i=0; i<4096; i++)
    ptable[i] = 0;

  pentry = 0 | 0x412;
  for (i=0; i<258; i++){
    ptable[i] = pentry;
    pentry += 0x100000;
  }

  // // UMODE VA mapping: Assume each proc has a 4MB Umode space
  // // ptable entry flag=|AP=11|0|dom1|1|CB10|=110|0001|1|1110|=0xC3E or 0xC32
  ptable[2048] = (0x800000 + (p->pid - 1)*0x100000)|0xC32;
  ptable[2049] = (0x1000000 + (p->pid - 1)*0x100000)|0xC32;
  ptable[2050] = (0x1800000 + (p->pid - 1)*0x100000)|0xC32;
  ptable[2051] = (0x2000000 + (p->pid - 1)*0x100000)|0xC32;

  for (i = 0; i < 4; i++)
  {
    PA = (char *)(running->pgdir[2048 + i] & 0xFFFF0000); // parent Umode PA
  CA = (char *)(p->pgdir[2048 + i] & 0xFFFF0000); // child Umode PA
    memcpy(CA, PA, 0x100000); // copy 4MB Umode image
  }

  for (i=1; i <= 14; i++)
  { // copy bottom 14 entries of kstack
    p->kstack[SSIZE-i] = running->kstack[SSIZE-i];
  }

  p->kstack[SSIZE - 14] = 0; // child return pid = 0
  p->kstack[SSIZE-15] = (int)goUmode; // child resumes to goUmode
  p->ksp = &(p->kstack[SSIZE-28]); // child saved ksp

  p->usp = running->usp; // same usp as parent
  p->cpsr = running->cpsr; // same spsr as parent
  // p->upc = running->upc; // same upc as parent - testing
  // p->kstack[SSIZE - 1] = VA(0);

  enqueue(&readyQueue, p);

  return p->pid;
}

int exec(char *cmdline) // cmdline=VA in Uspace
{
  int i, upa, usp;
  char *cp, kline[128], filename[32];
  PROC *p = running;

  strcpy(kline, cmdline); // fetch cmdline into kernel space
  
  // get first token of kline as filename
  cp = kline; i = 0;
  while(*cp != ' ' && *cp != 0) {
    filename[i] = *cp;
    i++; cp++;
  }

  filename[i] = 0;

  load(filename, p); 

  // copy cmdline to high end of Ustack in Umode image
  usp = VA(0x400000 - 128);
  strcpy((char *)usp, kline);
  p->usp = (int *)VA(0x400000 - 128);

  // fix syscall frame in kstack to return to VA=0 of new image
  for (i=2; i<14; i++) // clear Umode regs r1-r12
    p->kstack[SSIZE - i] = 0;

  p->kstack[SSIZE-1] = (int)VA(0); // return uLR = VA(0)

  return (int)p->usp; // will replace saved r0 in kstack
}