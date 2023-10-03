#define NPROC 9

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procsize = sizeof(PROC);

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
  if (running->status == READY)
     enqueue(&readyQueue, running);

  running = dequeue(&readyQueue);

  if (running->pid){
     color = running->pid;
  }
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

int my_ps()
{
  printf("PID\tPPID\tSTATUS\n");
  printf("---\t----\t------\n");
  for (int i = 0; i < NPROC; i++)
  {
    printf(" %d\t %d  \t", proc[i].pid, proc[i].ppid);

    printf("%s\n", str_status(&proc[i]));
  }
}