// wait.c file

extern PROC *freeList;
extern PROC *readyQueue;
extern PROC *sleepList;
extern PROC *running;

int ksleep(int event)
{
  int SR = int_off();
  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  tswitch();
  int_on(SR);
}

int kwakeup(int event)
{
  int SR = int_off();

  // remove all procs at front of sleep list where proc->event == event
  PROC* cur = sleepList;
  while (cur && cur->event == event)
  {
    sleepList = cur->next;

    cur->status = READY;
    enqueue(&readyQueue, cur);

    cur = sleepList;
  }

  // Preconditions:
  // 1) any proc->event == event is not at beginning of sleepList (TRUE b/c of above)
  // 2) cur = sleepList (TRUE b/c of above)
  PROC* prev = 0;
  while (cur)
  {
    if (cur->event == event)
    {
      prev->next = cur->next;

      cur->status = READY;
      enqueue(&readyQueue, cur);
    }

    prev = cur;
    cur = cur->next;
  }
  int_on(SR);
}

int kexit(int exitCode) 
{
  PROC* p1 = &proc[1];

  if (running->child)
  {
    // has at least one child
    // set parent of children as p1 
    PROC* cur = running->child;
    while (cur)
    {
      cur->parent = p1;
      cur = cur->sibling;
    }

    // add children list to end of p1 children list
    cur = p1->child;
    while (cur->sibling)
    {
      cur = cur->sibling;
    }

    cur->sibling = running->child;
  }

  running->exitCode = exitCode;
  running->status = ZOMBIE;
  kwakeup(running->parent);
  kwakeup(p1);
  tswitch();
}

int kwait(int *status)  
{
  if (running->child == 0)
  {
    return -1;
  }

  while (1)
  {
    PROC* prev = 0;
    PROC* cur = running->child;

    while (cur)
    {
      if (cur->status == ZOMBIE)
      {
        if (prev == 0)
        {
          // remove first child
          running->child = cur->sibling;
        }
        else
        {
          // remove child after the first
          prev->sibling = cur->sibling;
        }

        int pid = cur->pid;
        *status = cur->exitCode;
        cur->status = FREE;
        enqueue(&freeList, cur);
        return pid;
      }
      prev = cur;
      cur = cur->sibling;
    }
    ksleep(running);
  }
}