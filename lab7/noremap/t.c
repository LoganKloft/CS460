#include "type.h"
#include "string.c"

char *tab = "0123456789ABCDEF";
int BASE;
int color;

#include "kbd.c"
#include "vid.c"
#include "exceptions.c"
#include "queue.c"
#include "kernel.c"
#include "wait.c"
#include "fork.c"
#include "exec.c"
#include "svc.c"
#include "sdc.c"
#include "load.c"

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;

    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0xFFFF0000; // CHANGED BY LOGAN
   //  u32 *vectors_dst = (u32 *)0x80000000; // CHANGED BY LOGAN
   //  u32 *vectors_dst = (u32 *)0x0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

int kprintf(char *fmt, ...);

void DATA_handler()
{
  printf("data exception\n");
}

void IRQ_handler()
{
    int vicstatus, sicstatus;

    // read VIC status register to find out which interrupt
    vicstatus = VIC_STATUS;
    sicstatus = SIC_STATUS;  

    if (vicstatus & 0x80000000){
       if (sicstatus & 0x08){
          kbd_handler();
       }
       if (sicstatus & (1<<22)){
          sdc_handler();
       }
    }
}

extern int reset_handler();

int main()
{ 
   char line[128]; 
   
   color = RED;
   row = col = 0; 
   BASE = 10;
      
   fbuf_init();
   kprintf("                     Welcome to WANIX in Arm\n");
   kprintf("LCD display initialized : fbuf VA = %x\n", fb);
   color = CYAN;

   kbd_init();

   VIC_INTENABLE |= (1<<31);    // SIC to VIC's IRQ31
   /* enable KBD IRQ */
   SIC_ENSET |= (1<<3);  // KBD int=3 on SIC
   SIC_ENSET |= (1<<22);  // SDC int=22 on SIC

   sdc_init();
   
   color = RED;
   printf("reset_handler VA = %x\n", reset_handler);
   color = CYAN;
   kernel_init();

   kfork("u1");

   kprintf("P0 switch to P1 : enter a line : ");
   kgetline(line);

   while(1){
     unlock();
     if (readyQueue)
       tswitch();
   }
}
