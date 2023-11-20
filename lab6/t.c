
#include <stdint.h>
#include "type.h"
#include "string.c"

#define VA(x) (0x80000000 + (u32)x)

char *tab = "0123456789ABCDEF";
int BASE;
int color;

#include "kbd.c"
#include "vid.c"
#include "exceptions.c"
#include "queue.c"
#include "kernel.c"
#include "wait.c"
#include "memory.c"
#include "fork.c"
#include "svc.c"
#include "sdc.c"

void copy_vectors(void) {
    extern uint32_t vectors_start;
    extern uint32_t vectors_end;
    uint32_t *vectors_src = &vectors_start;
    uint32_t *vectors_dst = (uint32_t *)0;
    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

int mkPtable()
{
  int i;
  u32 *ut = (u32 *)0x4000;   // at 16KB
  u32 entry = 0 | 0x412;     // 0x412;// AP=01 (Kmode R|W; Umode NO) domaian=0
  for (i=0; i<4096; i++)
    ut[i] = 0;

  for (i=0; i<512; i++){     // for 512 MB real memory 
    ut[i] = entry;
    entry += 0x100000;
  }
}

int *mtable = (int *)0x4000; // initial pgdir at 16KB
int *ktable = (int *)0x8000; // new 2-level pgdir at 32KB
int makePageTable()
{
  int i, j;
  int *pgtable;
  int paddr;

  for (i=0; i<4096; i++){ // zero out 4096 entries of ktable[]
    ktable[i] = 0;
  }

  for (i=0; i<258; i++){ // ASSUME 256MB PA + 2MB I/O space
    ktable[i] = (0x500000 + i*1024) | 0x11; // DOMAIN 0,type=01
  }

  // build Kmode level-2 pgtables at 5MB
  for (i=0; i<258; i++){
    pgtable = (int *)(0x500000 + i*1024);
    paddr = i*0x100000 | 0x55E; // APs=01|01|01|01|CB=11|type=10

    for (j=0; j<256; j++){ // 256 entries, each points to 4KB PA
      pgtable[j] = paddr + j*4096;
    }
  }

  // build 64 level-1 pgdirs for other PROCs at 6MB
  ktable = (int *)0x600000; // build 64 proc's pgdir at 6M
  pgtable = (int*)0x8000;
  for (i=0; i<64; i++){ // 512KB area in 6MB
    ktable = (int *)(0x600000 + i*0x4000); // each pgdir 16KB

    for (j=0; j<4096; j++){ // copy ktable[ ]
      ktable[j] = pgtable[j];
    }
  }
  
  running->pgdir = ktable; // change P0's pgdir to ktable
  switchPgdir((int)ktable); // switch to 2-level pgdir
}

int kprintf(char *fmt, ...);

int cdata_handler()
{
  int saved_color = color;
  color = RED;
  u32 fault_status, fault_addr, domain, status;
  int spsr = get_spsr();
  kprintf("data_abort exception in ");
  if ((spsr & 0x1F) == 0x13)
    kprintf("SVC mode\n");
  if ((spsr & 0x1F) == 0x10)
    kprintf("USR mode\n");

  fault_status = get_fault_status();
  fault_addr = get_fault_addr();
  domain = (fault_status & 0xF0) >> 4;
  status = fault_status & 0xF;
  kprintf("status = %x: domain = %x status = %x ", fault_status, domain, status);

  if (status == 0x1) { // priority 1
    kprintf("(0x1 = alignment fault)\n");
  } else if (status == 0x4) { // priority 2
    kprintf("(0x4 = instruction cache maintenance fault)\n");
  } else if (status == 0xc) { // priority 3
    kprintf("(0xc = 1st level translation, synchronous external abort)\n");
  } else if (status == 0xe) { // priority 4
    kprintf("(0xe = 2nd level translation, synchronous external abort)\n");
  } else if (status == 0x5) { // priority 5
    kprintf("(0x5 = translation fault, section)\n");
  } else if (status == 0x7) { // priority 6
    kprintf("(0x7 = translation fault, page)\n");
  } else if (status == 0x3) { // priority 7
    kprintf("(0x3 = access flag fault, section)\n");
  } else if (status == 0x6) { // priority 8
    kprintf("(0x6 = access flag fault, page)\n");
  } else if (status == 0x9) { // priority 9
    kprintf("(0x9 = domain fault, section)\n");
  } else if (status == 0xb) { // priority 10
    kprintf("(0xb = domain fault, page)\n");
  } else if (status == 0xd) { // priority 11
    kprintf("(0xd = permission fault, section)\n");
  } else if (status == 0xf) { // priority 12
    kprintf("(0xf = permission fault, page)\n");
  } else if (status == 0x8) { // priority 13
    kprintf("(0x8 = synchronous external abort, nontranslation)\n");
  } else if (status == 0xa) { // priority 14
    // error in arm docs?
    // https://developer.arm.com/documentation/ddi0388/b/system-control-coprocessor/register-descriptions/c5--data-fault-status-register
    // 14 [3:0] should be 1010?
    // ofc I'm also looking at the wrong version :-)
    // and could also be identified using the upper bits
    // in the case that 0x10 is reserved
    kprintf("(0x1 = asynchronous external abort)\n");
  } else if (status == 0x2) { // priority 15
    kprintf("(0x1 = debug event)\n");
  }

  kprintf("VA addr = %x\n", fault_addr);
  color = saved_color;
}

// int cprefetch_abort_handler()
// {
//   kprintf("We are here\n");
// }

void IRQ_handler()
{
    int vicstatus, sicstatus;

    // read VIC status register to find out which interrupt
    vicstatus = VIC_STATUS;
    sicstatus = SIC_STATUS;  

    if (vicstatus & 0x80000000){
      if (sicstatus & (1<<3)){
          kbd_handler();
       }
       if (sicstatus & (1<<22)){
	   sdc_handler();
	 }
    }
}

int main()
{ 
   row = col = 0; 
   BASE = 10;
   color = RED;      
   fbuf_init();
   kprintf("                     Welcome to WANIX in Arm\n");
   kprintf("LCD display initialized : fbuf = %x\n", fb);
   color = WHITE;
   kbd_init();

   VIC_INTENABLE |= (1<<31);    // SIC to VIC's IRQ31
   
   /* enable KBD SDC IRQ */
   SIC_ENSET   |= (1<<3);   // KBD int=3 on SIC
   SIC_ENSET   |= (1<<22);  // SDC int=22 on SIC

   sdc_init();
   
   kernel_init();
   unlock();

   init_memory();
   makePageTable();
   kfork("u1");    // P1 Umode image = /bin/u1 file
   
   color = CYAN;
   kprintf("P0 switch to P1 : ");
   tswitch();  // switch to run P1 ==> never return again
}
