/*************** t.c file *************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

/***************************************************
        VIC, SIC registers are 4-bytes each
***************************************************/
u32 *VIC_BASE = 0x10140000;       // u32 pointer
#define VIC_STATUS     0x00/4
#define VIC_INTENABLE  0x10/4
#define VID_VADDR      0x30/4

u32 *SIC_BASE = 0x10003000;       // u32 pointer   
#define SIC_STATUS     0x00/4
#define SIC_ENSET      0x08/4
#define SIC_SOFTINTSET 0x10/4
#define SIC_PICENSET   0x20/4

#include "string.c"

int color;

#include "vid.c"
#include "kbd.c"
#include "timer.c"
#include "exceptions.c"

void copy_vectors(void)
{
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

void IRQ_chandler()
{
  u32 vicstatus = *(VIC_BASE + VIC_STATUS);  // read VIC status
  u32 sicstatus = *(SIC_BASE + SIC_STATUS);  // read SIC status
    
  if (vicstatus & (1<<4)){                   // timer0 at bit4 of VIC 
       timer_handler(0);
  }

  if (vicstatus & (1<<31)){     // VIC bit31 = SIC interrupts
     if (sicstatus & (1<<3)){   // KBD at bit3 on SIC
          kbd_handler();
     }
  }
}

int main()
{
   color = RED;
   row = col = 0; 
   fbuf_init();
   kbd_init();

   *(VIC_BASE + VIC_INTENABLE)  = 0x10;   // VIC bit4=1 to route timer0,1

 A3: // write code to set VIC bit-31 to route SIC IRQs at 31 of VIC
     // YOUR code here
    *(VIC_BASE + VIC_INTENABLE) |= (1 << 31);

   /***************
    #define SIC_intenable  0x08/4  // enabled interrupts: read-only 
    #define SIC_ENSET      0x08/4  // write to set SIC_intenable reg
   ****************/

 A4: // write code to to route KBD=IRQ3 at SIC
     // YOUR code here
    *(SIC_BASE + SIC_ENSET) |= (1 << 3);

   
   kputs("test TIMER KBD interrupt-driven drivers\n");
   timer_init();
   timer_start(0);
 
   color = CYAN;

 A5: // un-comment unlock(), and write code for unlock() in ts.s file
     unlock();
   
   kputs("in while(1) loop: enter keys from KBD\n");

   while(1);
}
