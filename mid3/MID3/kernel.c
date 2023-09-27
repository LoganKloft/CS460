// kernel.c file

#define NPROC 16
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procsize = sizeof(PROC);

int init()
{
  int i; 
  PROC *p;
  kprintf("kernel_init()\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = FREE;
     p->next = p + 1;
  }
  proc[NPROC-1].next = 0;
  freeList = &proc[0];

  sleepList = 0;
  readyQueue = 0;

  p = getproc(&freeList);
  p->status = READY;
  p->priority = 0;
  running = p;
  kprintf("running = proc %d\n", running->pid);
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
  //kprintf("proc %d in scheduler\n", running->pid);

  if (running->status==READY)
     enqueue(&readyQueue, running);

  running = dequeue(&readyQueue);

  u32 cpsr = getcpsr() & 0xFF;
  if (cpsr == 0x13 || cpsr == 0x93)
    mode = "SVC";
  if (cpsr == 0x12 || cpsr == 0x92)
    mode = "IRQ";
  
  running->time = 4;
  color = running->pid;

  printf("proc %d ISP=%x ksp=%x cpsr=%x mode=%s\n",
	  running->pid, ISP, running->ksp, cpsr, mode);
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
    kprintf("proc %d in body() input a char [s|f|x] : ", running->pid);
    c = kgetc(); 
    //printf("%c\n", c);

    switch(c){
      case 's': tswitch();           break;
      case 'f': kfork((int)body, 1); break;
      case 'x': kexit();             break;
    }
  }
}
