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

typedef volatile struct kbd{
  char *base;
  char buf[128];
  int head, tail;
  int data, room;
}KBD;

volatile KBD kbd;
int release = 0;

int kputc(char);

int kbd_init()
{
  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base + KCNTL) = 0x10; // bit4=Enable bit0=INT on
  *(kp->base + KCLK)  = 8;
  kp->head = kp->tail = 0;

  kp->data = 0; kp->room = 128;
  release = 0;
}


// kbd_handelr() for scan code set 2
void kbd_handler()
{
  u8 scode, c;
  KBD *kp = &kbd;
  char *mode;

  scode = *(kp->base + KDATA);

  if (scode == 0xF0){ // key release 
    release = 1;
    return;
  }
  
  if (release && scode != 0x12){ // ordinay key release
    release = 0;
    return;
  }

  c = ltab[scode];
  u32 cpsr = getcpsr() &0xFF;
  if (cpsr == 0x93)
    mode = "SVC";
  if (cpsr == 0x92)
    mode = "IRQ";
  
  printf("kbd interrupt: c=%x %c cpsr=%x mode=%s\n", c, c, cpsr, mode);

  kp->buf[kp->head++] = c;
  kp->head %= 128;

  kp->data++;
  kp->room--;
}

int kgetc()
{
  char c;
  KBD *kp = &kbd;
  
  while(kp->data == 0);

  lock();
   c = kp->buf[kp->tail++];
   kp->tail %= 128;
   kp->data--; kp->room++;
  unlock();
  return c;
}

int kgets(char s[ ])
{
  char c;
  KBD *kp = &kbd;
  
  while( (c = kgetc()) != '\r'){
    if (c=='\b'){
      s--;
      continue;
    }
    *s++ = c;
  }
  *s = 0;
  return strlen(s);
}
