
int body(), goUmode();
char *istring = "init start";

int *mtable = (int *)0x80500000; // P0's pgtable at 5MB

#define UPN 0
#define UVA UPN*0x100000

PROC *kfork(char *filename)
{
  int i; char *cp;
  u32 BA, Btop, Busp;

  PROC *p = getproc();
  if (p==0){
    kprintf("kfork failed\n");
    return (PROC *)0;
  }
  p->ppid = running->pid;
  p->parent = running;
  p->parent = running;
  p->status = READY;
  p->priority = 1;
  p->cpsr = (int *)0x10; // previous mode = UMODE
  
  p->pgdir = (int *)(0x80500000 + (p->pid)*0x4000); // at 5MB+pid*16K

  for (i=0; i<4096; i++){ // copy P0's pgdir entries
    p->pgdir[i] = mtable[i];
  }
  
  // only entry 0 is for UMODE image CHANGED BY LOGAN
  p->pgdir[UPN]=(0x800000 + (p->pid-1)*0x100000) | 0xC32; // entry 0 Umode

  load(filename, p); // p->PROC containing pid, pgdir, etc

  // set kstack to resume to body
  for (i=1; i<29; i++)  // all 28 cells = 0
    p->kstack[SSIZE-i] = 0;

  p->kstack[SSIZE-15] = (int)goUmode;  // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-28]);
 
  BA = p->pgdir[UPN] & 0xFFFF0000; // CHANGED BY LOGAN
  Btop = BA + 0x100000;
  Busp = Btop - 128 + 0x80000000;
  // printf("BA=%x Busp=%x\n", BA, Busp);

  cp = (char *)Busp;
  strcpy(cp, istring);
  printf("stack contents=%s UVA=%x\n", cp, 0);

  // KCW: for 1MB Umode image, ustack top is at VA=2MB, set it to 2MB-128
  cp = (char *)(0x200000 - 128);
  //printf("cp = %x\n", cp);

  p->kstack[SSIZE-14] = p->kstack[SSIZE-13] = (int)(cp); 
  p->usp = (int *)(cp);

  p->kstack[SSIZE-1] = (int)(UVA);

  enqueue(&readyQueue, p);

  printQ(readyQueue);
  kprintf("proc %d kforked a child %d\n", running->pid, p->pid); 
  return p;
}

int fork()
{
  // fork a child proc with identical Umode image=> same u1 file
  int i, j;
  char *cp, *cq;
  u32 *ucp, *upp;
  u32 PBA, PBtop, Pusp;
  u32 CBA, CBtop, Cusp;

  PROC *p = dequeue(&freeList);
  if (p==0){
    kprintf("fork failed\n");
    return -1;
  }
  p->ppid = running->pid;
  p->parent = running;
  p->status = READY;
  p->priority = 1;

  p->pgdir = (int *)(0x80500000 + (p->pid)*0x4000); // at 5MB+pid*16K

  for (i=0; i<4096; i++){     // copy P0's pgdir entries
    p->pgdir[i] = mtable[i];
  }
  
  // entry 1 is for UMODE image CHANGED BY LOGAN
  p->pgdir[UPN]=(0x800000 + (p->pid-1)*0x100000) | 0xC32; // entry 1 Umode    

  //printf("running usp=%x linkR=%x\n", running->usp, running->upc);

  PBA = (running->pgdir[UPN] & 0xFFFF0000); // CHANGED BY LOGAN
  PBA += 0x80000000;
  PBtop = PBA + 0x100000;  // top of 1MB Uimage

  printf("FORK: parent %d uimage at %x\n", running->pid, PBA);
 
  CBA = (p->pgdir[UPN] & 0xFFFF0000); // CHANGED BY LOGAN
  CBA += 0x80000000;
  CBtop = CBA + 0x100000;  // top of 1MB Uimage
  printf("FORK: child  %d uimage at %x\n", p->pid, CBA);

  printf("copy Umode image from %x to %x\n", PBA, CBA);
  // copy 1MB of Umode image
  upp = (int *)PBA;
  ucp = (int *)CBA;
  for (i=0; i<0x100000/4; i++){
     *ucp++ = *upp++;
  }
  p->upc = running->upc;
  p->usp = running->usp;   // both should be VA in their sections
  p->cpsr = running->cpsr;
  
  // the hard part: child must resume to the same place as parent
  // child kstack must contain |parent kstack|goUmode stack|=> copy kstack
  printf("copy kernel mode stack\n");
  j = &running->kstack[SSIZE] - running->ksp;
 
  for (i=1; i <= j; i++){
    p->kstack[SSIZE-i] = running->kstack[SSIZE-i];
    //printf("%x ", p->kstack[SSIZE-i]);
  }
   //printf("\n");

  printf("FIX UP child resume PC to %x\n", running->upc);
  p->kstack[SSIZE-1] = (int)(running->upc);
  p->kstack[SSIZE-(j+1)] = (int)goUmode;
  p->ksp = &(p->kstack[SSIZE-(j+14)]);
 
  enqueue(&readyQueue, p);

  kprintf("KERNEL: proc %d forked a child %d\n", running->pid, p->pid); 
  printQ(readyQueue);

  return p->pid;
}
