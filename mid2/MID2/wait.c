// wait.c file

extern PROC *freeList;
extern PROC *readyQueue;
extern PROC *sleepList;
extern PROC *running;

int ksleep(int event)
{
  // implement this
}

int kwakeup(int event)
{
  // implement this
}

int kexit(int exitCode) 
{
  // implement this
}

int kwait(int *status)  
{
  // implement this
}
