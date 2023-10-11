
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

  // build p's pgtable 
  p->pgdir = (int *)(0x600000 + (p->pid - 1)*0x4000);
  ptable = p->pgdir;

  // initialize pgtable
  for (i=0; i<4096; i++)
    ptable[i] = 0;

  pentry = 0x412;
  for (i=0; i<258; i++){
    ptable[i] = pentry;
    pentry += 0x100000;
  }

  // UMODE VA mapping: Assume each proc has a 1MB Umode space at 8MB+(pid-1)*1MB
  // ptable entry flag=|AP=11|0|dom1|1|CB10|=110|0001|1|1110|=0xC3E or 0xC32    

  ptable[2048] = 0x800000 + (p->pid - 1)*0x100000|0xC32;   // CB=00
  // entries 2049 to 4095 all 0 for INVALID

  // load filename to Umode image area at 7MB+(pid-1)*1MB
  
  load(filename, p); // p->PROC containing pid, pgdir, etc
  
  // kstack must contain a syscall frame FOLLOWed by a resume frame
  for (i=1; i<29; i++)  // all 28 cells = 0
     p->kstack[SSIZE-i] = 0;

  // p->kstack[SSIZE-1] = (int)0x80000000;
  p->kstack[SSIZE-1] = VA(0);

  p->kstack[SSIZE-15] = (int)goUmode;

  p->ksp = &(p->kstack[SSIZE-28]);

  // p->usp = (int *)(0x80100000);
  p->usp = (int *)VA(0x100000);
  p->cpsr = (int *)0x10;    // saved cpsr = previous mode was Umode

  enqueue(&readyQueue, p);

  kprintf("proc %d kforked a child %d: ", running->pid, p->pid); 
  printQ(readyQueue);

  return p->pid;
}
