
extern PROC *kfork();
PROC proc[NPROC], *freeList, *readyQueue, *sleepList, *running;
int procsize = sizeof(PROC);

char *pname[NPROC]={"sun", "mercury", "venus", "earth", "mars", "jupiter",
                    "saturn","uranus","neptune"};

int kernel_init()
{
  PROC *p; 
  int i, j; 
  int *MTABLE, *mtable;

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
  running = getproc();
  running->status = READY;

  printList(freeList);
  printQ(readyQueue);

  printf("building P0's pgdirs at 5MB\n");
  running->pgdir = 0x80500000;  

  MTABLE = (int *)0x80004000;   // initial Mtable at PA=0x4000
  mtable = (int *)0x80500000;   // mtable at 5MB

  for (j=0; j<4096; j++)        // clear mtable entries to 0
    mtable[j] = 0;

  mtable[0] = MTABLE[0]; // NEED the low 1MB for vectors and handler entries

  for (j=2048; j<2048+258; j++){  // last 258 entries copy from MTABLE
      mtable[j] = MTABLE[j];
  }

  printf("CLEAR pgdir entry 1-256 to 0\n");
  mtable = (int *)0x80500000;
  for (i=1; i<256; i++){ // only entry 0 and 2048-4095 are valid
      MTABLE[i] = 0;
      mtable[i] = 0;
  }

  printf("switch to P0's pgdir at 5MB : ");
  switchPgdir((int)0x500000);
  printf("OK\n");
}

int scheduler()
{
  char line[8];
  int pid; PROC *old=running;
  char *cp;
  u32 newpgdir;
  int currentPgdir;

  kprintf("proc %d in scheduler ", running->pid);
  if (running->status==READY)
     enqueue(&readyQueue, running);
  //printQ(readyQueue);
  running = dequeue(&readyQueue);

  kprintf("next running = %d\n", running->pid);
  color = running->pid;

  // must switch to new running's pgdir; possibly need also flush TLB
  if (running != old){
    //currentPgdir = getpgdir() + 0x80000000;
    //printf("currentPgdir=%x\n", currentPgdir);
    printf("switch to proc %d pgdir at %x ", running->pid, running->pgdir);
    newpgdir = (u32)running->pgdir - 0x80000000;
    printf("pgdir[1] = %x\n", running->pgdir[1]);
 
    switchPgdir(newpgdir);

    printf("OK\n");

    //currentPgdir = getpgdir() + 0x80000000;
    //printf("currentPgdir=%x OK\n", currentPgdir);
    /*
    cp = (char *)0x100000; // Umode image begin at 1MB
    printf("string=%s\n",cp);
    cp = (char *)0x80800000;
    printf("string=%s\n",cp);
    **/
  }
}  

int do_tswitch()
{
  tswitch();
}

int do_sleep()
{
  int event;
  printf("enter an event value to sleep : ");
  event = geti();
  ksleep(event);
}

int do_wakeup()
{
  int event;
  printf("enter an event value to wakeup : ");
  event = geti();
  kwakeup(event);
}

int do_exit()
{
  int value;
  if (running->pid == 1){
    printf("P1 should not die\n");
    return -1;
  }
  printf("enter an exit value : ");
  value = geti();
  kexit(value);
}


int do_wait()
{
  int pid, status;
  printf("proc %d wait for ZOMBIE child\n", running->pid);
  pid = kwait(&status);
  printf("proc %d waited for a dead child=%d ", running->pid, pid);
  if (pid>0){
    printf("status=%x%d", status, status);
  }
  printf("\n");
}

int do_kfork()
{
  kfork("/bin/u1");
}

int body()
{
  char c; char line[64];
  int pid;
  kprintf("proc %d resume to body()\n", running->pid);
  while(1){
    pid = running->pid;
    if (pid==1) color=RED;
    if (pid==2) color=GREEN;
    if (pid==3) color=CYAN;
    if (pid==4) color=PURPLE;
    if (pid==5) color=YELLOW;
    if (pid==6) color=BLUE;   
    if (pid==7) color=WHITE;

    printf("----------------------------------------------\n");
    printList(freeList);
    printQ(readyQueue);
    printSleepList(sleepList);
    printf("----------------------------------------------\n");
    kprintf("proc %d in body(), parent = %d, input a char [s|f|q|z|a|w|u] : ", 
	    running->pid, running->ppid);
    kprintf("pidaddr=%x\n", &pid);
    kgetline(line);
    c = line[0];
 
    switch(c){
      case 's': do_tswitch(); break;
      case 'f': do_kfork();   break;
      case 'q': do_exit();    break;
 
      case 'z': do_sleep();   break;
      case 'a': do_wakeup();  break;
      case 'w': do_wait();    break;
      case 'u': do_goUmode();   break;
    }
  }
}


int do_goUmode()
{
  char line[32];
  int i; 
  int *usp = running->usp;

  kprintf("%d goUmode: usp=%x\n", running->pid, usp);
  for (i=0; i<8+14; i++){
    kprintf("%x ", usp[i]);
  }
  kprintf("enter a line ");
  kgetline(line); 
  goUmode(); 
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
char *pstatus[]={"FREE   ","READY  ","SLEEP  ","BLOCK  ","ZOMBIE ", "RUN   "};
int kps()
{
  int i; PROC *p; 
  for (i=0; i<NPROC; i++){
     p = &proc[i];
     kprintf("proc[%d]: pid=%d ppid=%d", i, p->pid, p->ppid);
     if (p==running)
       printf("%s ", pstatus[5]);
     else
       printf("%s", pstatus[p->status]);
     printf("name=%s\n", p->name);
  }
}

int kchname(char *s)
{ 
  kprintf("kchname: name=%s\n", s);
  strcpy(running->name, s);
  return 123;
}
int kkfork()
{
  PROC *p = kfork("/bin/u1");
  if (p)
    return p->pid;
  return -1;
}

int kkwait(int *status)
{
    int pid, e; 
    pid = kwait(&e);
    printf("write %x to status at %x in Umode\n", e, status);
    *status = e;
    return pid;
}

int ktswitch()
{
  kprintf("%d in ktswitch()\n", running->pid);
  tswitch();
  kprintf("%d exit ktswitch()\n", running->pid);
}

int getusp()
{
  return (int)running->usp;
}
