int color;

#include "type.h"
#include "vid.c"
#include "string.c"
#include "queue.c"
#include "exceptions.c"
#include "kbd.c"
#include "kernel.c"
#include "pv.c"
#include "wait.c"
#include "timer.c"
#include "pipe.c"

PIPE *kpipe;

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

int kprintf(char *fmt, ...);

void IRQ_handler()
{
    int vicstatus, sicstatus;
    vicstatus = VIC_STATUS; // VIC_STATUS=0x10140000=status reg
    sicstatus = SIC_STATUS;  

   if (vicstatus & (1<<4)){
      timer_handler();
   }

   if (vicstatus & (1 << 31)){
      if (sicstatus & (1 << 3)){
         kbd_handler();
      }
   }
}

int pipe_writer() // pipe writer task code
{
   char line[128];
   int n;
   char c;

   while(1)
   {
      kprintf("P%d> Enter a line to get : ", running->pid);

      kgets(line);
      kprintf("\n");

      kprintf("P%d> write line=[%s] to pipe\n", running->pid, line);
      
      n = write_pipe(kpipe, line, strlen(line));
      printf("P%d> wrote %d bytes to pipe\n", running->pid, n);

      kprintf("WRITER continue or exit: [c|e] ");
      c = kgetc();
      kprintf("\n");
      if (c == 'e')
      {
         kpipe->nwriter--;
         V(&kpipe->data);
         break;
      }
   }

   kprintf("WRITER exit\n");
   kexit(1);
}

int pipe_reader() // pipe reader task code
{
   char line[128];
   int i, n;
   char c;

   while(1)
   {
      kprintf("P%d> read from pipe\n", running->pid);

      n = read_pipe(kpipe, line, 8);
      kprintf("P%d> READ n=%d bytes from pipe : [", running->pid, n);

      for (i=0; i<n; i++)
         kputc(line[i]);

      kprintf("]\n");

      kprintf("READER continue or exit: [c|e] ");
      c = kgetc();
      kprintf("\n");
      if (c == 'e')
      {
         kpipe->nreader--;
         V(&kpipe->room);
         break;
      }
   }

   kprintf("READER exit\n");
   kexit(1);
}

int main()
{ 
   color = WHITE;
   row = col = 0; 

   fbuf_init();
   kbd_init();
   timer_init();
   pipe_init();
   kernel_init();
   
   // allow timer interrupts
   VIC_INTENABLE |= (1<<4);  // timer0,1 at bit4

   // allow KBD interrupts   
   VIC_INTENABLE |= (1<<31); // allow VIC IRQ31
   SIC_INTENABLE |= (1<<3);  // KBD int=3 on SIC

   kpipe = create_pipe();

   kfork((int)pipe_writer, 1);
   kfork((int)pipe_reader, 1);

   timer_start();

   while(1){
     if (readyQueue)
        tswitch();
   }
}
