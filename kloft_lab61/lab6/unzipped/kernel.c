/********************
#define  SSIZE 1024
#define  NPROC  9
#define  FREE   0
#define  READY  1
#define  SLEEP  2
#define  BLOCK  3
#define  ZOMBIE 4
#define  printf  kprintf
 
typedef struct proc{
  struct proc *next;   //  0
  int    *ksp;         //  4

  int    *usp;         //  8    saved Umode usp
  int    *upc;         // 12                upc
  int    *cpsr;        // 16                cpsr

  int    *pgdir;       // level-1 pgtable

  int    status;
  int    priority;
  int    pid;
  int    ppid;
  struct proc *parent;
  int    event;
  int    exitCode;

  char   name[32];
  int    kstack[SSIZE];
}PROC;
***************************/
extern int kfork();
PROC proc[NPROC], *freeList, *readyQueue, *sleepList, *running;

int procsize = sizeof(PROC);

char *pname[NPROC]={"sun", "mercury", "venus", "earth", "mars", "jupiter",
                     "saturn","uranus","neptune"};

u32 *MTABLE = (u32 *)0x4000;
int kernel_init()
{
  int i, j; 
  PROC *p; char *cp;
  int *MTABLE, *mtable;
  int paddr;

  kprintf("kernel_init()\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = FREE;
    p->priority = 0;
    p->ppid = 0;
    strcpy(p->name, pname[i]);
    p->next = p + 1;
  }
  proc[NPROC-1].next = 0;
  freeList = &proc[0];
  readyQueue = 0;
  sleepList = 0;

  running = dequeue(&freeList);
  running->status = READY;
  running->pgdir = 0x4000;   // P0's pgdir at 16KB
  printList(freeList);
}

int scheduler()
{
  char line[8];
  int pid; PROC *old=running;
  char *cp;
  kprintf("proc %d in scheduler\n", running->pid);
  if (running->status==READY)
     enqueue(&readyQueue, running);
  printQ(readyQueue);
  running = dequeue(&readyQueue);

  kprintf("next running = %d\n", running->pid);

  // must switch to new running's pgdir; possibly need also flush TLB
  color = running->pid;
  if (running != old){
    printf("switch to proc %d pgdir at %x\n", running->pid, running->pgdir);
    kprintf("up0 = %x up1 = %x up2 = %x up3 = %x\n", running->pgdir[2048], running->pgdir[2049], running->pgdir[2050], running->pgdir[2051]);
    switchPgdir((u32)running->pgdir);
  }
}  

int kgetpid()
{
  //kprintf("kgetpid: pid = %d\n", running->pid);
  return running->pid;
}

int kgetppid()
{
  //kprintf("kgetppid: pppid = %d\n", running->ppid);
  return running->ppid;
}

char* str_status(PROC* p)
{
  if (p == running)
  {
    return "RUNNING";
  }
  else
  {
    switch (p->status)
    {
      case FREE:
        return "FREE";
        break;
      case READY:
        return "READY";
        break;
      case SLEEP:
        return "SLEEP";
        break;
      case BLOCK:
        return "BLOCK";
        break;
      case ZOMBIE:
        return "ZOMBIE";
        break;
    }
  }

  return "ERROR";
}

int kps()
{
  printf("PID\tPPID\tSTATUS\tNAME\n");
  printf("---\t----\t------\t----\n");
  for (int i = 0; i < NPROC; i++)
  {
    printf(" %d\t %d  \t", proc[i].pid, proc[i].ppid);

    printf("%s\t%s\n", str_status(&proc[i]), proc[i].name);
  }
}

int kchname(char *s)
{ 
  kprintf("kchname: name=%s\n", s);
  strcpy(running->name, s);
  return 123;
}

int kgetPA()
{
  return running->pgdir[2048] & 0xFFFF0000;
}