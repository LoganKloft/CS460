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

TIMER timer;           // 4 timers; 2 timers per unit; at 0x00 and 0x20

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
}

int timer_handler() {
    TIMER *t = &timer;

    t->tick++; 
    if (t->tick >= 60){ // every second
       t->tick = 0;
       running->time--;
       printf("proc %d time=%d\n", running->pid, running->time);

      if (running->time == 0)
      {
        timer_clearInterrupt();
        return 1;
      }

    }
    // clear timer interrupt
    timer_clearInterrupt();
    return 0;
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
