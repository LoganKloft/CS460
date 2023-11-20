
	.text
.code 32
.global reset_handler
.global vectors_start, vectors_end
.global proc, procsize
.global tswitch, scheduler, running, goUmode
.global getcsr
.global switchPgdir
.global irq_stack
.global lock, unlock, int_off, int_on, getpgdir

reset_handler:
	
  mov r0, #0x4000
  mov r1, #4096              // r1=4096
  mov r2, #0                 // r2=0
1: // ZERO OUT FIRST 4096 ENTRIES STARTING AT 0x4000
  str r2, [r0], #0x04        // store r2 to [r0]; inc r0 by 4
  subs r1,r1, #1             // r1-- 
  bgt 1b                     // loop r1=4096 times

  mov r0, #0x4000
  mov r1, #256
  add r1, r1, #2             // r1=258
  mov r2, #0x100000          // r2=1M
  mov r3, #(0x1 << 10)       // r3=AP=01 (KRW, user no) AP=11: both KU r/w
  orr r3, r3, #0x12          // r3 = 0x412 OR 0xC12 if AP=11: used 0xC12
// mov r3, #0x412 // should be SAME but not allowed by ARM's # value 
2: // 1:1 MAPPING STARTING AT 0x4000 FOR 258 - THIS IS KERNEL AT LOW END
  str r3, [r0],#0x04         // store r3 to [r0]; inc r0 by 4
  add r3, r3, r2             // inc r3 by 1M
  subs r1,r1,#1              // r1-- 
  bgt 2b                     // loop r1=258 times

//******** map kernel VA to 2GB ************
// 1:1 MAPPING STARTING AT 2048 FOR 258 - THIS IS KERNEL AT HIGH END
  mov r0, #0x4000
  add r0, r0,#(2048*4)       // add 8K for 2048th entry
  mov r2, #0x100000          // r2 = 1MB increments
  mov r1, #256               // 258 entries
  add r1,r1,#2
  mov r3, #(0x1 << 10)       // r3=AP=01 (KRW, user no) AP=11: both KU r/w
  orr r3, r3, #0x12          // r3 = 0x412 OR 0xC12 if AP=11: used 0xC12
3:	
  str r3, [r0], #0x04        // store r3 to [r0]; inc r0 by 4
  add r3, r3, r2             // inc r3 by 1M
  subs r1,r1, #1             // r1-- 
  bgt 3b                     // loop r1=256 times

  @ // ADDED BY LOGAN :-)
  @ mov r0, #0x4000
  @ add r0, r0,#(4096*4)
  @ sub r0, r0,#4
  @ mov r3, #(0x1 << 10)
  @ orr r3, r3, #0x12
  @ str r3, [r0]
  @ // ktable[4095] = 0 | 0x412;

   // ADDED BY LOGAN :-)
  mov r0, #0x4000           // start at page table
  add r0, r0,#(4096*4)      // go to pgtable[4096] which does not exist
  sub r0, r0,#4             // go to pgtable[4095] which is the last entry in the page table
  mov r3, #0x8000           // then we convert this into level 2 pgtable at 32 KiB
  orr r3, r3, #0x11         // DOMAIN 0, type = 01
  str r3, [r0]

  mov r0, #0x8000            // start at level-2 page table at 32 KiB
  add r0, r0, #(0xF0*4)      // go to pgtable[240]
  mov r3, #0                 // create entry for PA 0x0
  ldr r4, =#0x55e           
  orr r3, r3, r4             // APs=01|01|01|01|CB=11|type=10
  str r3, [r0]
  // END
	
  mov r0, #0x4000
  mcr p15, 0, r0, c2, c0, 0  // set TTBase with PHYSICAL address 0x4000
  mcr p15, 0, r0, c8, c7, 0  // flush TLB

  // set domain: all 01=client(check permission) 11=master(no check)
  mov r0, #0x1               // 01 for client mode
  mcr p15, 0, r0, c3, c0, 0  // write to domain REG c3

  // enable MMU 
  mrc p15, 0, r0, c1, c0, 0   // read control REG to r0
  @ orr r0, r0, #0x00002001     // set bit0 of r0 to 1 - AND VECTOR REMAPPED TO HIGH END [13] = 1
  @ ldr r0, =0x00002001     // set bit0 of r0 to 1 - AND VECTOR REMAPPED TO HIGH END [13] = 1
  ldr r0, =0b10000000000001     // set bit0 of r0 to 1 - AND VECTOR REMAPPED TO HIGH END [13] = 1
  // https://developer.arm.com/documentation/ddi0333/h/system-control-coprocessor/system-control-processor-registers/c1--control-register?lang=en 
  mcr p15, 0, r0, c1, c0, 0   // write to control REG c1 ==> MMU on
  nop
  nop
  nop
  mrc p15, 0, r2, c2, c0, 0   // read TLB base reg c2 into r2
  mov r2, r2                  // ??   

  adr pc, start
start:	 
  MSR cpsr, #0x13
  LDR sp, =svc_stack_top
	
  /* go in IRQ mode to set IRQ stack and enable IRQ interrupts */
  MSR cpsr, #0x12            // write to cspr, so in IRQ mode now 
  ldr sp, =irq_stack_top
	
  /* go back to SVC mode */
  MSR cpsr, #0x13    // write to cspr, so in SVC mode now

  // set previous mode of SVC mode to USER mode
  MRS r0, spsr       // get previous mode SR in r0
  BIC r1, r0, #0x1F  // clear lowest 5 bits
  ORR r1, r1, #0x10  // 10000 = USERMODE
  MSR spsr, r1       // write to previous mode spsr
	
// still in SVC mode
// Enable IRQ interrupts by unmasking IF bits in cpsr register: 
//      |             |76543210|
// cspr=|NZCV.........|IFTmodes| ==> IF are interrupt MASK bits:1=masked out
  mov r0, r2         // restore r0 to SVC cpsr
  BIC r0, r0, #0xC0  // I and F bits=0 enable IRQ,FIQ
  MSR cpsr, r0       // I and F interrupts are enables 

  /* copy vector table to address 0 */
  BL copy_vectors
 
  adr r0, mainstart
  ldr pc, [r0]
  B .
	
.align 4
Mtable:	    .word 0x4000
mainstart:  .word main

irq_handler:     
  sub	lr, lr, #4
  stmfd	sp!, {r0-r12, lr}  // save all Umode regs in kstack

  bl	IRQ_handler  // call IRQ_handler() in C in svc.c file   

  ldmfd	sp!, {r0-r12, pc}^ // pop from kstack but restore Umode SR

data_handler:
  sub	lr, lr, #4
  stmfd	sp!, {r0-r12, lr}

  bl	DATA_handler  

  ldmfd	sp!, {r0-r12, pc}^


tswitch: // tswitch() in Kmode
// IRQ off
  MRS r0, cpsr
  ORR r0, r0, #0x80
  MSR cpsr, r0
	
  stmfd	sp!, {r0-r12, lr}

  LDR r0, =running // r0=&running
  LDR r1, [r0,#0]  // r1->runningPROC
  str sp, [r1,#4]  // running->ksp = sp

  bl	scheduler

  LDR r0, =running
  LDR r1, [r0,#0]     // r1->runningPROC
  lDR sp, [r1,#4]     // sp = running->ksp

// IRQ on
  MRS r0, cpsr
  BIC r0, r0, #0x80
  MSR cpsr, r0	

  ldmfd	sp!, {r0-r12, pc}
	
	
svc_entry:
	
   stmfd sp!, {r0-r12, lr}  // save ALL regs!!!! => goUmode: restore ALL !!!

   ldr r5, =running   // r5=&running
   ldr r6, [r5, #0]   // r6 -> PROC of running

   mrs r4, spsr
   str r4, [r6, #16]

// to SYS mode to get usp=r13 from USER mode
   mrs r2, cpsr       // r2 = SVC mode cpsr
   mov r3, r2         // save a copy in r3
   orr r2, r2, #0x1F  // r0 = SYS mode
   msr cpsr, r2       // change cpsr in SYS mode	

// now in SYS mode, r13 same as User mode sp r14=user mode lr
   str sp, [r6, #8]   // save usp into proc.usp at offset 8
   str lr, [r6, #12]  // save return PC into proc.ups at offset 12

// change back to SVC mode
   msr cpsr, r3

// save user mode usp into proc.usp at offest 8
   str sp, [r6, #4]    // running->ksp = sp

// enable interrupts
   MRS r4, cpsr
   BIC r4, r4, #0xC0  // I and F bits=0 enable IRQ,FIQ
   MSR cpsr, r4

  bl	svc_handler  // svc.c fixed uR0 for return value to Umode 

goUmode:
// IRQ off
   MRS r0, cpsr
   mov r1, r0
   ORR r1, r1, #0x80
   MSR cpsr, r1

   ldr r5, =running   // r5=&running
   ldr r6, [r5, #0]   // r6 -> PROC of running

   ldr r4, [r6, #16]  // get saved SPSR
   msr spsr, r4       // restore spsr
	
// set CPSR to SYS mode to access user mode sp	
   mrs r2, cpsr       // r2 = SVC mode cpsr
   mov r3, r2         // save a copy in r3
   orr r2, r2, #0x1F  // r0 = SYS mode
   msr cpsr, r2       // change cpsr to SYS mode	

// now in SYS mode   
   ldr sp, [r6, #8]   // set Umode sp = running->usp

// back to SVC mode	
   msr cpsr, r3       // back to SVC mode
	
// IRQ on              // optional: will return to Umode with saved cpsr 
   MRS r0, cpsr
   BIC r0, r0, #0x80
   MSR cpsr, r0

// ^: pop regs from kstack BUT also copy spsr into cpsr ==> back to Umode
   ldmfd sp!, {r0-r12, pc}^ // ^ : pop kstack AND to previous mode

int_off:	// SR = int_off() in C
  MRS r0, cpsr
  mov r1, r0
  ORR r1, r1, #0x80
  MSR cpsr, r1
  mov pc, lr

int_on:	        // int_off(SR) in C
  MSR cpsr, r0
  mov pc, lr
	
unlock:
  MRS r0, cpsr
  BIC r0, r0, #0x80
  MSR cpsr, r0
  mov pc,lr	

lock:
  MRS r0, cpsr
  ORR r0, r0, #0x80
  MSR cpsr, r0
  mov pc,lr	

getcsr:
   mrs r0, cpsr
   mov pc, lr

switchPgdir:	// switch pgdir to new PROC's pgdir; passed in r0
  // r0 contains address of PROC's pgdir address	
  mcr p15, 0, r0, c2, c0, 0  // set TTBase to C2
  mov r1, #0
  mcr p15, 0, r1, c8, c7, 0  // flush TLB 
  mcr p15, 0, r1, c7, c10, 0 // flush TLB
  mrc p15, 0, r2, c2, c0, 0  // read TLB base reg C2
	
  // set domain: all 01=client(check permission) 11=master(no check)
  mov r0, #0x5                // 0101 for D1D0=client mode
  mcr p15, 0, r0, c3, c0, 0   // write 0x3 to domain reg C3
	
  mov pc, lr                  // return


getpgdir: // read tlb base register C2
    mrc p15, 0, r0, c2,c0, 0  // read P15's C2 into r0
    mov pc,lr              // return

vectors_start:
  LDR PC, reset_handler_addr
  LDR PC, undef_handler_addr
  LDR PC, svc_handler_addr
  LDR PC, prefetch_abort_handler_addr
  LDR PC, data_abort_handler_addr
  B .
  LDR PC, irq_handler_addr
  LDR PC, fiq_handler_addr

reset_handler_addr:          .word 0x80000000 + reset_handler
undef_handler_addr:          .word 0x80000000 + undef_handler
svc_handler_addr:            .word 0x80000000 + svc_entry
prefetch_abort_handler_addr: .word 0x80000000 + prefetch_abort_handler
data_abort_handler_addr:     .word 0x80000000 + data_handler
irq_handler_addr:            .word 0x80000000 + irq_handler
fiq_handler_addr:            .word 0x80000000 + fiq_handler

vectors_end:


	.end
