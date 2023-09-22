// timer.c file
// timer register offsets from base address

/** u32 * offsets **/
#define TLOAD   0x0
#define TVALUE  0x1
#define TCNTL   0x2
#define TINTCLR 0x3
#define TRIS    0x4
#define TMIS    0x5
#define TBGLOAD 0x6

typedef struct timer{
  u32 *base;            // timer's base address; as u32 pointer
  int tick, hh, mm, ss; // per timer data area
  char clock[16]; 
}TIMER;

TIMER timer; 

typedef struct timerQueueEntry {
  int time;
  PROC* proc;
  struct timerQueueEntry* next;
} TQE;

TQE timerQueueEntries[NPROC];
TQE* timerQueue = 0;

void timer_init()
{
  int i;
  TIMER *tp;
  kprintf("timer_init()\n");

    tp = &timer;
    tp->base = (u32 *)0x101E2000; 
    *(tp->base+TLOAD) = 0x0;   // reset
    *(tp->base+TVALUE)= 0xFFFFFFFF;
    *(tp->base+TRIS)  = 0x0;
    *(tp->base+TMIS)  = 0x0;
    *(tp->base+TLOAD) = 0x100;
    *(tp->base+TCNTL) = 0x62; //011-0000=|NOTEn|Pe|IntE|-|scal=00|32-bit|0=wrap|
    *(tp->base+TBGLOAD) = 0xF000/4;

  tp->tick = tp->hh = tp->mm = tp->ss = 0;
  strcpy(tp->clock, "00:00:00");
  displayClock(tp->clock);
}

void decrement_and_remove_tqe()
{
  if (timerQueue)
  {
    TQE* tqe = timerQueue;
    tqe->time -= 1;

    while (timerQueue && tqe->time == 0)
    {
      timerQueue = timerQueue->next;
      kwakeup(tqe->proc);
      tqe = timerQueue;
    }
  }
}

void print_tqe()
{
  TQE* tqe = timerQueue;
  printf("timerQueue = ");
  if (tqe == 0)
  {
    printf("NULL");
  }
  while (tqe)
  {
    printf("[P%d %d] => ", tqe->proc->pid, tqe->time);
    tqe = tqe->next;
  }
  printf("\n");
}

// clock = hh:mm:ss
//         01234567
void updateClock() {
  TIMER *t = &timer;

  t->ss = t->ss + 1;
  if (t->ss == 60) {
    t->ss = 0;
    t->mm = t->mm + 1;
  } 
  if (t->mm == 60) {
    t->mm = 0;
    t->hh = t->hh + 1;
  }

  // update hours char display
  int tens = t->ss / 10;
  int ones = t->ss % 10;
  t->clock[7] = '0' + ones;
  t->clock[6] = '0' + tens;

  // update minutes char display
  tens = t->mm / 10;
  ones = t->mm % 10;
  t->clock[4] = '0' + ones;
  t->clock[3] = '0' + tens;

  // update seconds char display
  tens = t->hh / 10;
  ones = t->hh % 10;
  t->clock[1] = '0' + ones;
  t->clock[0] = '0' + tens;
}

void timer_handler()
{
    TIMER *t = &timer;
    t->tick++; 
    if (t->tick >= 60){   // second
       t->tick = 0;

      updateClock();

      if (timerQueue)
      {
      print_tqe();
      }

      displayClock(t->clock);

      decrement_and_remove_tqe();
    }
    
    timer_clearInterrupt(0);
}
    
void timer_start() // timer_start(0), 1, etc.
{
  TIMER *tp = &timer;

  kprintf("timer_start base=%x\n", tp->base);
  *(tp->base+TCNTL) |= 0x80;  // set enable bit 7
}

int timer_clearInterrupt() // timer_start(0), 1, etc.
{
  TIMER *tp = &timer;
  *(tp->base+TINTCLR) = 0xFFFFFFFF;
}

void timer_stop() // timer_start(0), 1, etc.
{
  TIMER *tp = &timer;
  *(tp->base+TCNTL) &= 0x7F;  // clear enable bit 7
}

void add_tqe(int time, PROC* proc)
{
  TQE* tqe = &timerQueueEntries[proc->pid];
  tqe->time = time;
  tqe->proc = proc;
  tqe->next = 0;


  // timerQueue is empty
  if (timerQueue == 0)
  {
    timerQueue = tqe;
    return;
  }

  TQE* prev = 0;
  TQE* cur = timerQueue;
  tqe->time -= cur->time;
  while (cur != 0 && tqe->time >= 0)
  {
    prev = cur;
    cur = cur->next;

    if (cur != 0)
    {
      tqe->time -= cur->time;
    }
  }

  // insert at the beginning
  if (prev == 0)
  {
    tqe->time += cur->time; // undo subtraction
    tqe->next = cur;
    cur->time = cur->time - tqe->time; // next entry is relative to timer of new entry
    timerQueue = tqe;
    return;
  }

  // insert at the end
  if (cur == 0)
  {
    prev->next = tqe;
    return;
  }

  // insert between beginning and end
  tqe->time += cur->time; // undo subtraction
  prev->next = tqe;
  tqe->next = cur;
  cur->time -= tqe->time; // next entry is relative to timer of new entry
}

// enter into queue
int kitimer(int time)
{
  int truffle = int_off();

  add_tqe(time, running);
  printf("PROC %d go to sleep in kitimer\n", running->pid);
  ksleep(running);

  int_on(truffle); 
}
