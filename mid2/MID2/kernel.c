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
  p->sibling = 0;
  p->child = 0;

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
  // kprintf("proc %d in scheduler ", running->pid);
  if (running->status == READY)
     enqueue(&readyQueue, running);
  running = dequeue(&readyQueue);
  // kprintf("next running = %d\n", running->pid);
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

// code of processes
int body()
{
  char c, cmd[64];

  printf("proc %d resume to body()\n", running->pid);

  while(1){
    printf("proc %d running  parent=%d\n", running->pid, running->ppid);

    // write code to print childList=[pid, status]->...->NULL
    PROC* cur = running->child;
    printf("childList =");
    while (cur)
    {
      char *status;
      printf("[%d, %s]->", cur->pid, str_status(cur));
      cur = cur->sibling;
    }
    printf("NULL\n");
    
    printList("freeList  ", freeList);
    printList("readyQueue", readyQueue);
    printsleepList(sleepList);
	
    printf("Enter a command [ps|switch|fork|sleep|wakeup|exit|wait] : ");
    kgets(cmd);
    printf("\n");
    
    if (strcmp(cmd, "switch")==0)
    {
      tswitch();
    }
    else if (strcmp(cmd, "fork")==0)
    {
      kfork((int)body, 1);
    }
    else if (strcmp(cmd, "ps") == 0)
    {
      my_ps();
    }
    else if (strcmp(cmd, "sleep") == 0)
    {
      if (running->pid == 1)
      {
        printf("P1 can't sleep by command\n");
      }
      else
      {
        printf("input an event value to sleep : ");
        kgets(cmd);
        printf("\n");
        ksleep(atoi(cmd));
      }
    }
    else if (strcmp(cmd, "wakeup") == 0)
    {
      printf("input an event value to wakeup : ");
      kgets(cmd);
      printf("\n");
      kwakeup(atoi(cmd));
    }
    else if (strcmp(cmd, "exit") == 0)
    {
      if (running->pid == 1)
      {
        printf("P1 never die\n");
      }
      else
      {
        printf("input an exitCode value : ");
        kgets(cmd);
        printf("\n");
        kexit(atoi(cmd));
      }
    }
    else if (strcmp(cmd, "wait") == 0)
    {
      int status = -1;
      int pid = kwait(&status);

      if (pid == -1)
      {
        printf("wait error : no child\n");
      }
      else
      {
        printf("P%d waited for a ZOMBIE P%d status=%d\n", running->pid, pid, status);
      }
    }
  }
}
