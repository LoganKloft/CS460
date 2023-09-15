// kernel.c file

#define NPROC 9
/*********** in type.h ***************
typedef struct proc{
  struct proc *next;
  int    *ksp;

  int    pid;
  int    ppid;
  int    priority;
  int    status;
  int    event;
  int    exitCode;

  struct proc *parent;
  struct proc *child;
  struct proc *sibling;
  
  int    kstack[SSIZE];
}PROC;
**************************************/
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procsize = sizeof(PROC);

int body();

int kernel_init()
{
  int i; 
  PROC *p;
  kprintf("kernel_init()\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->ppid = 0;
    p->status = FREE;
    p->next = p + 1;
  }
  proc[NPROC-1].next = 0; 
  freeList = &proc[0];    // freeList = ALL free procs 
  printList("freeList", freeList);
  
  readyQueue = 0;
  sleepList = 0;
  
  // creat P0 as running process
  p = dequeue(&freeList); // take proc[0] off freeList
  p->priority = 0;
  p->status = READY;
  p->ppid = 0;
  p->parent = p;
  p->child = p->sibling = 0;
  running = p;           // running -> proc[0] with pid=0

  kprintf("running = %d\n", running->pid);
  printList("freeList", freeList);
}

int kfork(int func, int priority)
{
  int i;
  PROC *p = dequeue(&freeList);
  if (p==0){
    printf("no more PROC, kfork failed\n");
    return 0;
  }
  p->status = READY;
  p->priority = priority;
  p->ppid = running->pid;
  p->parent = running;
  p->child = p->sibling = 0;
  
  for (i=1; i<15; i++)
      p->kstack[SSIZE-i] = 0;

  p->kstack[SSIZE-1] = (int)func;  // saved regs in dec address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-14]); // saved sp in proc.ksp

  enqueue(&readyQueue, p);

  printf("proc %d kforked a child %d\n", running->pid, p->pid);
  printList("readyQueue", readyQueue);
  return p->pid;
}

int scheduler()
{
  // kprintf("proc %d in scheduler ", running->pid);
  if (running->status == READY)
     enqueue(&readyQueue, running);
  running = dequeue(&readyQueue);
  // kprintf("next running = %d\n", running->pid);
  if (running->pid){
     color = running->pid;
  }
}

// code of processes
int body()
{
  char c, cmd[64];

  printf("proc %d resume to body()\n", running->pid);

  while(1){
    printf("proc %d running  parent=%d\n", running->pid, running->ppid);

    // write code to print childList=[pid, status]->...->NULL
    
    printList("freeList  ", freeList);
    printList("readyQueue", readyQueue);
    printsleepList(sleepList);
	
    printf("Enter a command [switch|kfork|myfork] : ");
    kgets(cmd);
    printf("\n");
    
    if (strcmp(cmd, "switch")==0)
       tswitch();
    else if (strcmp(cmd, "kfork")==0)
      kfork(body, 1);
    else if (strcmp(cmd, "myfork")==0){
      A();
    }
  }
}

int myfork()
{
  printf("proc %d enter myfork()\n", running->pid);

  // get lr of running proc and call kfork(lr, 1)
  // int lr = get_lr();
  // kfork(lr, 1);
  
  // fork a process
  int cpid = kfork(0, 1);

  // (1)
  // copy running process kstack to forked process kstack
  for (int i = 0; i < SSIZE; i++)
  {
    proc[cpid].kstack[i] = running->kstack[i];
  }

  // (2)
  // want fp to be the value of fp on myfork's stack (which is an address that points to location of lr in previous function)
  int* fp = get_fp();

  // want lr to be the value of lr on myfork's stack
  int lr = get_lr();
  
  // (3)
  // add tswitch requirements to end of D
  int* sp = get_sp(); // sp points to last local of D in running proc
  sp = sp - running->kstack + proc[cpid].kstack; // convert to forked proc
  for (int i = 1; i < 15; i++)
  {
    *(sp - i) = 0;
  }
  *(sp - 1) = lr;
  *(sp - 3) = fp;
  proc[cpid].ksp = sp - 14;

  // (4)
  // fix the fp's
  fp = sp - 3; // we should be updating *fp, then going to *fp
  
  // once fp points to fp on the stack
  while (fp != &proc[cpid].kstack[SSIZE - 1] - 1)
  {
    // fp is pointing at fp
    int* next = *fp;
    *fp = next - running->kstack + proc[cpid].kstack;

    // advance fp to next fp
    fp = *fp - 4; 
  }
  
  printf("proc %d exit  myfork()\n", running->pid);

  return cpid;
}

// LR - stores address of where PC should resume (what instruction)
// FP - stores address of previous FP
// a = 1 - local variable
// b = 2 - local variable
// <-- SP the top of the stack that is being used 
  
int A()
{
  int a,b;
  a = 1; b=2;
  printf("proc%d enter A()\n", running->pid);
  B();
  printf("proc%d exit A() a=%d b=%d\n", running->pid, a, b);
}

int B()
{
  int c,d;
  c=3; d=4;
  printf("proc%d enter B()\n", running->pid);
  C();  
  printf("proc%d exit B() c=%d d=%d\n", running->pid, c, d);
}

int C()
{
  int e,f;
  e=5; f=6;
  printf("proc%d enter C()\n", running->pid);
  D();
  printf("proc%d exit C() e=%d f=%d\n", running->pid, e, f);
}

int D()
{
  int g,h;
  g=7; h=8;
  printf("proc%d enter D()\n", running->pid);
  int cid = myfork();
  printf("proc%d exit D() g=%d h=%d\n", running->pid, g, h);
}

  
