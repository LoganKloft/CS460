// timer.c file

// timer registers are 4-byte

/******* u32 offsets *******/
#define TLOAD   0x00/4
#define TVALUE  0x04/4
#define TCNTL   0x08/4
#define TINTCLR 0x0C/4
#define TRIS    0x10/4
#define TMIS    0x14/4
#define TBGLOAD 0x18/4

typedef struct timer{
  u32 *base;            // timer's base address; u32 pointer

  int tick, hh, mm, ss; // per timer data area
  char clock[16]; 
}TIMER;

TIMER timer;            // timer struct

void timer_init()
{
  int i;
  TIMER *tp;
  kputs("timer_init()\n");

  tp = &timer;
  tp->base = (u32 *)0x101E2000; 
 
  *(tp->base+TLOAD) = 0x0;             // reset
  *(tp->base+TVALUE)= 0x0;             // initial counter value
  *(tp->base+TRIS)  = 0x0;
  *(tp->base+TMIS)  = 0x0;
  *(tp->base+TBGLOAD) = 0xE0000/60;    // reload timer count

/************************************
 timer Control Register at base+TCNTL
    7      6       5        4         3    2     1          0
 Enable Periodic IntEable   -  scale= 0    0  32-bit(1) WrapAound(0)
*****************************************/

  // program timer control register: periodic, 32-bit counter, wraparound
  *(tp->base+TCNTL) = 0x42; //010-0010=|En|Pe|IntE|-|scale=00|32-bit|0=wrap|

A1: // write code to enable timer interrupts (set bit5)
    // YOUR code here
  *(tp->base + TCNTL) |= (1 << 5);
  
  tp->tick = tp->hh = tp->mm = tp->ss = 0;
  strcpy((char *)tp->clock, "00:00:00");
  displayClock(tp->clock);
}

int cursor_on = 0;
void timer_handler(){
  
    TIMER *t = &timer;

    t->tick++;
    if (t->tick == 60){
      t->tick = 0;

      // draw cursor
      if (cursor_on) {
        cursor_on = 0;
        clrcursor();
      } else {
        cursor_on = 1;
        putcursor();
      }

      updateClock();
      displayClock(t->clock);
    }
    
    timer_clearInterrupt();
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

void timer_start() 
{
  TIMER *tp = &timer;

  *(tp->base+TCNTL) |= 0x80;  
}

int timer_clearInterrupt() 
{
  TIMER *tp = &timer;
  *(tp->base+TINTCLR) = 0xFFFFFFFF;
}

void timer_stop()
{
  TIMER *tp = &timer;
  *(tp->base+TCNTL) &= 0x7F;  // clear enable bit 7
}
