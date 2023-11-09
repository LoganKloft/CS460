
#include "keymap2"

#define LSHIFT 0x12
#define RSHIFT 0x59
#define ENTER  0x5A
#define LCTRL  0x14
#define RCTRL  0xE014

#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10
extern PROC *running;

typedef volatile struct kbd{
  char *base;
  char buf[128];
  int head, tail, data, room;
}KBD;

int kputc(char);

volatile KBD kbd;
int release = 0;
volatile int kline;

int kbd_init()
{
  KBD *kp = &kbd;
  kp->base = (char *)VA(0x10006000);
  *(kp->base + KCNTL) = 0x10; // bit4=Enable bit0=INT on
  *(kp->base + KCLK)  = 8;
  kp->head = kp->tail = 0;
  kp->data = 0; kp->room = 128;

  release = 0;
}

void kbd_handler()
{
  u8 scode, c;
  KBD *kp = &kbd;
  //  color = YELLOW;
  scode = *(kp->base + KDATA);
  //  printf("scanCode = %x  ", scode);

  if (scode == 0xF0){ // key release 
    release = 1;
    return;
  }
  
  if (release && scode != 0x12){ // ordinay key release
    release = 0;
    return;
  }

  c = ltab[scode];

  if (c != '\r')
     kputc(c);

  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;

  if (c=='\r')
    kline++;
}

int kgetc()
{
  char c;
  KBD *kp = &kbd;
  //printf("%d in kgetc\n", running->pid); 
  unlock();
  while(kp->data == 0);

  lock();
  c = kp->buf[kp->tail++];
  kp->tail %= 128;
  kp->data--; kp->room++;
  unlock();
  return c;
}

int kgetline(char s[ ])
{
  char c;
  KBD *kp = &kbd;
  
  if (kline==0){
    //kprintf("enter a line from KBD: ");
     while(kline==0); // wait until kline > 0
  }
  // fetch a line from kbuf[ ] 

  while(1){
      c = kp->buf[kp->tail++];
      *s++ = c;
      kp->tail %= 128;
      kp->data--; kp->room++;
      if (c=='\r')
	break;
  }
  *(s-1) = 0;
  kline--;
}

