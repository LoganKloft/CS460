/*************** t.c file ****************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

/**************************************
  timer register are 4-byte each
**************************************/
u32 *VIC_BASE = 0x10140000;    // u32 pointer
#define VIC_STATUS    0x00/4
#define VIC_INTENABLE 0x10/4
#define VIC_VADDR     0x30/4

int color;

#include "string.c"
#include "vid.c"
#include "timer.c"
#include "exceptions.c"

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;

    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

void IRQ_chandler()
{
    // read VIC status register to find out which interrupt occurred
    u32 vicstatus = *(VIC_BASE + VIC_STATUS);

    if (vicstatus & (1<<4)){   // bit4 for timer0,1
       timer_handler();
    }
}

int main()
{
   color = YELLOW;
   row = col = 0; 
   fbuf_init();

   /**********************************************************************
        VIC_INTENABLE register: each bit i=1 enables IRQi
   ****************************************************************/
   
A2: // write code to enable VIC to route IRQ4 of timer0,1
    // YOUR code here
    *(VIC_BASE + VIC_INTENABLE) |= (1 << 4);
 
   kputs("test timer driver by interrupts\n");
   timer_init();        // initialize timer driver    
   timer_start();       // start timer

A3: // un-comment unlock(), and write code for unlock() at A1 in ts.s file
   unlock();      // mask in IRQ to allow CPU to take IRQ interrupts 

   kputs("Enter while(1) loop\n");
   while(1);
}
