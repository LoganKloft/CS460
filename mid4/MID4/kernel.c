extern PROC *getproc();
extern PROC *dequeue();
extern int kitimer();

#define NPROC 16
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procsize = sizeof(PROC);
int body();

int init()
{
  int i, j; 
  PROC *p;
  kprintf("kernel_init()\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = READY;
     p->next = p + 1;
  }
  proc[NPROC-1].next = 0;
  freeList = &proc[0];

  sleepList = 0;
  readyQueue = 0;

  running = getproc(&freeList);
  running->priority = 0;
  kprintf("running = %d\n", running->pid);
}

int kfork(int func, int priority)
{
  int i;
  PROC *p = getproc(&freeList);
  if (p==0){
    kprintf("kfork failed\n");
    return (PROC *)0;
  }
  p->ppid = running->pid;
  p->parent = running;
  p->parent = running;
  p->status = READY;
  p->priority = priority;
  
  // set kstack to resume to body
  for (i=1; i<15; i++)
    p->kstack[SSIZE-i] = 0;
  p->kstack[SSIZE-1] = (int)func;  // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE-14]);
 
  enqueue(&readyQueue, p);
  //printQ(readyQueue);
  kprintf("proc %d kforked a child %d\n", running->pid, p->pid); 

  return p->pid;
}

int scheduler()
{
  char *mode;

  if (running->status==READY)
     enqueue(&readyQueue, running);

  running = dequeue(&readyQueue);

  u32 cpsr = getcpsr() & 0x1F;
  if (cpsr == 0x13)
    mode = "SVC";
  if (cpsr == 0x12)
    mode = "IRQ";
  
  running->time = 128;
  color = running->pid;

  printf("proc %d ISP=%x ksp=%x mode=%s\n", running->pid, ISP, running->ksp);
}  

int ksleep(int event)
{
  int sr = int_off();
  running->event = event;
  running->status = SLEEP;
  tswitch();
  int_on(sr);
}

int kwakeup(int event)
{
  int i;
  PROC *p;
  int sr = int_off();
  for (i=1; i<NPROC; i++){
    p = &proc[i];
    if (p->status==SLEEP && p->event==event){
      p->status = READY;
      enqueue(&readyQueue, p);
      printf("wakeup PROC %d\n", p->pid);
      printf("proc %d running ", running->pid);
      printQ(readyQueue);
    }
  }
  int_on(sr);
}

void kexit()
{
  printf("proc %d kexit\n", running->pid);
  running->status = FREE;
  putproc(&freeList, running);
  tswitch();
}
    
int body()
{
  char c; 
  kprintf("proc %d resume to body()\n", running->pid);

  while(1){
    unlock();
    printf("input time value for proc %d : ", running->pid);
    int t = geti();
    printf("\n");
    kitimer(t);
    printf("proc %d running again\n", running->pid);
  }
}
