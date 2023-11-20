#include "type.h"
#include "string.c"

u32 ISP;

#include "queue.c"
#include "kbd.c"
#include "vid.c"
#include "exceptions.c"
#include "kernel.c"
#include "timer.c"

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;

    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

int IRQ_handler()
{
    int vicstatus, sicstatus;
    // read VIC SIV status registers to find out which interrupt
    vicstatus = VIC_STATUS;
    sicstatus = SIC_STATUS;  

    if (vicstatus & (1<<4)){
         return timer_handler();
    }
    if (vicstatus & (1<<31)){ // SIC interrupts=bit_31=>KBD at bit 3 
      if (sicstatus & (1<<3)){
          kbd_handler();
       }
    }

    return 0;
}

int main()
{ 
   int i; 
   color = WHITE;
   row = col = 0; 
   
   fbuf_init();
   kprintf("Welcome to Wanix in ARM\n");
   kbd_init();
   
   // enable timer0 SIC IRQ
   VIC_INTENABLE |= (1<<4);  // timer0,1 at bit4 
   VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31
   // enable KBD IRQ 
   SIC_ENSET     |= (1<<3);  // KBD int=3 on SIC

   timer_init();

   init();

   extern u32 irq_stack_top;
   ISP = &irq_stack_top;
   u32 cpsr = getcpsr();
   printf("ISP=%x cpsr=%x\n", ISP, cpsr);

   kprintf("P0 kfork tasks\n");
   for (i=1; i<=4; i++){
      kfork((int)body, 1);
   }
   printQ(readyQueue);
   
   timer_start();   
   unlock();
   while(1){
     if (readyQueue)
        tswitch();
   }
}

