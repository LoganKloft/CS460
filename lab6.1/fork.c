
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

// Assume: Umode image size in MB
#define PSIZE 0x400000
PROC *fork1()
{
  int i, j, npgdir, npgtable;
  int *pgtable;

  // create a new proc *p: SAME AS BEFORE
  PROC *p = dequeue(&freeList);
  if (p==0){
    kprintf("fork1 failed\n");
    return (PROC *)0;
  }

  p->pgdir = (int *)(0x600000 + (p->pid-1)*0x4000); // same as before

  // copy entries from ktable at 32KB
  int *ktable = (int *)0x8000;
  for (i=0; i<4096; i++){
    p->pgdir[i] = ktable[i];
  }

  p->psize = 0x400000; // Uimage size in MB
  npgdir = p->psize/0x100000; // no. of pgdir entries and pgtables
  npgtable = npgdir;

  // fill in pgdir entries and construct Umode pgtables
  for (i=0; i<npgdir; i++){
    pgtable = (int *)palloc(); // allocate a page but only use 1KB
    p->pgdir[2048+i] = (int)pgtable | 0x31; // pgdir entry

    for (j=0; j<256; j++){ // pgtable
      pgtable[j] = (int)((int)palloc() | 0xFFE);
    }
  }

  return p;
}

int kfork(char *filename)
{
  int i; 
  int pentry, *ptable;

  PROC *p = fork1();
  if (p==0){
    kprintf("kfork failed\n");
    return (PROC *)0;
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

  // load filename to Umode image area at 7MB+(pid-1)*1MB
  load(filename, p); // p->PROC containing pid, pgdir, etc
  
  // kstack must contain a syscall frame FOLLOWed by a resume frame
  for (i=1; i<29; i++)  // all 28 cells = 0
     p->kstack[SSIZE-i] = 0;

  p->kstack[SSIZE-1] = VA(0);
  p->kstack[SSIZE-15] = (int)goUmode;
  p->ksp = &(p->kstack[SSIZE-28]);
  p->usp = (int *)VA(0x400000);


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

// copy parent's page frames to child' page frames
void copyimage(PROC *parent, PROC *child)
{
  int i, j;
  int *ppgtable, *cpgtable, *cpa, *ppa;
  int npgdir = parent->psize/0x100000;

  for (i=0; i < npgdir; i++){ // each image has npgdir page tables
    ppgtable = (int *)(parent->pgdir[i+2048] & 0xFFFFFC00);
    cpgtable = (int *) (child->pgdir[i+2048] & 0xFFFFFC00);

    for (j=0; j<256; j++){ // copy page table frames
      ppa = (int *)(ppgtable[j] & 0xFFFFF000);
      cpa = (int *)(cpgtable[j] & 0xFFFFF000);
      memcpy((char *)cpa, (char *)ppa, 4096); // copy 4KB page frame
    }
  }
}

// works if set p->pgdir = running->pgdir
// runs if set kstack[SSIZE - 1] = VA(0) (but this is like exec u1)
int fork()
{
  int i;
  int pentry, *ptable;
  char *PA, *CA;

  PROC *p = fork1();
  if (p==0)
  { 
    kprintf("fork failed\n"); return -1; 
  }

  kprintf("%x %x %x %x\n", p->pgdir[2048], p->pgdir[2049], p->pgdir[2050], p->pgdir[2051]);

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

  copyimage(running, p);

  for (i=1; i <= 14; i++)
  { // copy bottom 14 entries of kstack
    p->kstack[SSIZE-i] = running->kstack[SSIZE-i];
  }

  p->kstack[SSIZE - 14] = 0; // child return pid = 0
  p->kstack[SSIZE-15] = (int)goUmode; // child resumes to goUmode
  p->ksp = &(p->kstack[SSIZE-28]); // child saved ksp

  p->usp = running->usp; // same usp as parent
  p->cpsr = running->cpsr; // same spsr as parent

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