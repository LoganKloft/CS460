// queue.c file

PROC *getproc(PROC **list)
{
  PROC *p = *list;
  if (p){
    *list = p->next;
  }
  return p;
}

int putproc(PROC **list, PROC *p)
{
  p->next = *list;
  *list = p;
}

int enqueue(PROC **queue, PROC *p)
{
  PROC *q  = *queue;
  if (q==0){
    *queue = p;
    p->next = 0;
    return;
  }
  if ((*queue)->priority < p->priority){
    p->next = *queue;
    *queue = p;
    return;
  }
  while (q->next && p->priority <= q->next->priority){
    q = q->next;
  }
  p->next = q->next;
  q->next = p;
}

PROC *dequeue(PROC **queue)
{
  PROC *p = *queue;
  if (p)
    *queue = p->next;
  return p;
}

int printQ(PROC *p)
{
  kprintf("readyQueue = ");
  while(p){
    kprintf("[%d %d]->", p->pid,p->priority);
    p = p->next;
  }
  kprintf("NULL\n");
}



int printSleepList(PROC *p)
{
  printf("sleepList   = ");
   while(p){
     kprintf("[%d %d]->", p->pid,p->event);
     p = p->next;
  }
  kprintf("NULL\n"); 
}

int printList(PROC *p)
{
   kprintf("freeLis    = ");
   while(p){
     kprintf("[%d]->", p->pid);
     p = p->next;
  }
  kprintf("NULL\n"); 
}

