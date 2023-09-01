#include "keymap2"
/**********************************************
KBD is a char device: each register is a bytes
**********************************************/
#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10

typedef volatile struct kbd{
  char *base;     // base address as char *
  char buf[128];
  int head, tail, data, room;
}KBD;

KBD kbd;

int release;            // key release flag
int upper;              // upper = 0 = lowercase, upper = 1 = uppercase
int ctrl_flag;          // control held flag

int kbd_init()
{
    KBD *kp = &kbd;
    kp->base = (char *)0x10006000;

    *(kp->base + KCLK)  = 8;       // ARM manual says clock=8
    *(kp->base + KCNTL) = 0x02;    // control reg bit2 = KBD on

 A2: // write code to enable KBD interrupts (bit4=INTenable)
     // YOUR CODE here
     *(kp->base + KCNTL) |= (1 << 4);
  
    kp->head = kp->tail = 0;       // circular buffer char buf[128]
    kp->data = 0; kp->room = 128;

    release = 0;
    ctrl_flag = 0;
    upper = 0;
}

void kbd_handler()
{
  u8 scode, c;
  KBD *kp = &kbd;
  color = YELLOW;

  scode = *(kp->base + KDATA);   // get scan code of this interrpt

  if (scode == 0xF0){  // it's key release 
     release = 1;      // set release flag
     
    return;
  }

  if (release == 1){   // scan code after 0xF0
     release = 0;      // reset release flag

    // shift released
    if (scode == 0x12) {
      upper = upper == 0 ? 1 : 0; // swap upper state
      kputs("Shift released\n");
    }

    // ctrl released
    if (scode == 0x14) {
      ctrl_flag = 0;
      kputs("Control released\n");
    }

     return;
  }

  // shift pressed
  if (scode == 0x12) {
    upper = upper == 0 ? 1 : 0;
    kputs("Shift pressed\n");
    return;
  }

  // ctrl pressed
  if (scode == 0x14) {
    ctrl_flag = 1;
    kputs("Control pressed\n");
    return;
  }

  // caps lock pressed
  if (scode == 0x58) {
    upper = upper == 0 ? 1 : 0;
    kputs("Caps Lock pressed\n");
    return;
  }

  // handle Control-C
  if (ctrl_flag == 1 && scode == 0x21) {
    kputs("Control-C key\n");
    return;
  }

  // handle Control-D
  if (ctrl_flag == 1 && scode == 0x23) {
    kputs("Control-D key\n");
    c = 0x4;
  }
  else if (upper == 1) {
    // map scode to ASCII in uppercase 
    c = utab[scode];
  } else {
    // map scode to ASCII in lowercase 
    c = ltab[scode];
  }

  kputs("kbd interrupt : ");
  if (c != '\r')
    kputc(c);
  kputs("\n");

  kp->buf[kp->head++] = c;
  kp->head %= 128;
  kp->data++; kp->room--;
}

int kgetc()
{
  char c;
  int sr;
  KBD *kp = &kbd;

  unlock();                          // unmask IRQ in case it was masked out
  while(kp->data == 0);              // BUSY wait while kp->data is 0 

  sr = int_off();                   // mask out IRQ, sr=original cpsr
    c = kp->buf[kp->tail++]; 
    kp->tail %= 128;                 /*** Critical Region ***/
    kp->data--; kp->room++;
  int_on(sr);                      // restore original cpsr
    
  return c;
}

int kgets(char s[ ])
{
  char c;
  while( (c = kgetc()) != '\r'){
    *s = c;
    s++;
  }
  *s = 0;
}

