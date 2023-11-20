	.text
.code 32
.global reset_handler, data_handler
.global vectors_start, vectors_end
.global proc, procsize
.global tswitch, scheduler, running, goUmode
.global int_off, int_on, lock, unlock, getcsr
.global switchPgdir
.global get_fault_status, get_fault_addr, get_spsr, get_prefetch_addr, get_prefetch_status
	
//.set Mtable, 0x4000  // level-1 page table must be at 16K boundary
/********************************************************
mode:	USER: 10000  0x10
	FIQ : 10001  0x11
	IRQ : 10010  0x12
	SVC : 10011  0x13
        ABT : 10111  0x17
	UND : 11011  0x1B
	SYS : 11111  0x1F
********************************************************/
reset_handler:
  /* set SVC stack to HIGH END of proc[0].kstack[] */
  LDR sp, =svc_stack_top	

  /* go in IRQ mode to set IRQ stack and enable IRQ interrupts */
  MSR cpsr, #0xD2
  LDR sp, =irq_stack_top  // set IRQ stack poiner

  /* go in ABT mode to set ABT stack */
  MSR cpsr, #0xD7
  LDR sp, =abt_stack_top

  /* go in UND mode to set UND stack */
  MSR cpsr, #0xDB
  LDR sp, =und_stack_top

  /* go back in SVC mode */
  MSR cpsr, #0x13      // write to cspr, so in SVC mode now

  // set previous mode of SVC mode to USER mode with interrupts ON
  MSR spsr, #0x10

  /* copy vector table to address 0 */
  BL copy_vectors

  BL mkPtable                 // build pagetable in C; much easier

  ldr r0, Mtable
  mcr p15, 0, r0, c2, c0, 0  // set TTBase
  mcr p15, 0, r0, c8, c7, 0  // flush TLB

  // set domain0: 01=client(check permission) 11=master(no check)
  mov r0, #0x5               // both domain0,1 AP=0101 for CLIENT
  mcr p15, 0, r0, c3, c0, 0
	
// enable MMU
  mrc p15, 0, r0, c1, c0, 0
  orr r0, r0, #0x00000001     // set bit0
  mcr p15, 0, r0, c1, c0, 0   // write to c1
  nop
  nop
  nop
  mrc p15, 0, r2, c2, c0
  mov r2, r2

  adr r0, mainstart
  ldr pc, [r0]
	
  B .
	
.align 4
Mtable:	    .word 0x4000
mainstart:  .word main
	
.align 4

irq_handler:         // IRQ handler entry point
  sub	lr, lr, #4   
  stmfd	sp!, {r0-r12, lr}  // save all Umode regs in kstack
  bl	IRQ_handler  // call IRQ_handler() in C
  ldmfd	sp!, {r0-r12, pc}^ // pop from kstack but restore Umode SR

data_handler:
  sub lr, lr, #4
  stmfd sp!, {r0-r12, lr}
  bl cdata_handler
  ldmfd sp!, {r0-r12, pc}^

@ prefetch_abort_handler:
@   @ from https://developer.arm.com/documentation/dui0471/m/handling-processor-exceptions/prefetch-abort-handler
@   sub lr, lr, #4
@   stmfd sp!, {r0-r12, lr}
@   bl cprefetch_abort_handler
@   ldmfd sp!, {r0-r12, pc}^

tswitch: // tswitch() in Kmode

  stmfd	sp!, {r0-r12, lr}

// mask out IRQ
  MRS r7, cpsr
  ORR r7, r7, #0x80
  MSR cpsr, r7

  LDR r0, =running          // r0=&running
  LDR r1, [r0, #0]          // r1->runningPROC
  str sp, [r1, #4]          // running->ksp = sp

  bl	scheduler

  LDR r0, =running
  LDR r1, [r0, #0]          // r1->runningPROC
  lDR sp, [r1, #4]          // sp = running->ksp

// mask in IRQ
  MRS r7, cpsr
  BIC r7, r7, #0x80
  MSR cpsr, r7

  ldmfd	sp!, {r0-r12, pc}  // all in Kmode

svc_entry:

   stmfd sp!, {r0-r12, lr}
	
// save Umode upc, usp, cpsr in proc.usp, proc.usp, proc.cpsr
   ldr r4, =running   // r4=&running
   ldr r5, [r4, #0]   // r5 -> PROC of running

   mrs r6, spsr
   str r6, [r5, #12]  // save spsr into PROC.spsr

// change to SYS mode to get Umode usp, ulr
   mrs r6, cpsr       // r6 = SVC mode cpsr
   mov r7, r6         // save a copy in r7
   orr r6, r6, #0x1F  // r0 = SYS mode
   msr cpsr, r6       // change cpsr in SYS mode	

// now in SYS mode, r13 same as User mode sp r14=user mode lr
   str sp, [r5, #8]   // save usp into proc.usp at offset 8

// change back to SVC mode
   msr cpsr, r7

// enable IRQ interrupts while handle syscall
   MRS r6, cpsr
   BIC r6, r6, #0xC0  // I and F bits=0 enable IRQ,FIQ
   MSR cpsr, r6

  bl	svc_handler  

goUmode:
	
// IRQ off
  MRS r7, cpsr
  ORR r7, r7, #0x80
  MSR cpsr, r7

  ldr r4, =running   // r4=&running
  ldr r5, [r4, #0]   // r5 -> PROC of running

  ldr r6, [r5, #12]
  msr spsr, r6       // restore spsr = saved Umode cpsr
 	
// change to SYS mode
  mrs r6, cpsr       // r6 = SVC mode cpsr
  mov r7, r6         // save a copy in r7
  orr r6, r6, #0x1F  // r6 = SYS mode
  msr cpsr, r6       // change cpsr to SYS mode	

// now in SYS mode: restore usp and cpsr
   ldr sp, [r5, #8]   // restore usp from PROC.usp
  
   msr cpsr, r7       // back to SVC mode
	
// mask in IRQ  // not necessary because will return to Umode with saved cpsr
  MRS r7, cpsr
  BIC r7, r7, #0x80
  MSR cpsr, r7

// ^: pop regs from kstack BUT also copy spsr into cpsr ==> back to Umode
   ldmfd sp!, {r0-r12, pc}^ // ^ : pop kstack AND to previous mode

int_off:          // SR = int_off()
  MRS r0, cpsr
  mov r1, r0
  ORR r1, r1, #0x80
  MSR cpsr, r1    // return value in r0 = original cpsr
  mov pc,lr	

int_on:           // int_on(SR);  SR in r0
  MSR cpsr, r0
  mov pc,lr	

unlock:
  MRS r0, cpsr
  BIC r0, r0, #0x80
  MSR cpsr, r0
  mov pc, lr	

lock:
  MRS r0, cpsr
  ORR r0, r0, #0x80
  MSR cpsr, r0
  mov pc, lr	

getcsr:
   mrs r0, cpsr
   mov pc, lr
	
vectors_start:
  LDR PC, reset_handler_addr
  LDR PC, undef_handler_addr
  LDR PC, svc_handler_addr
  LDR PC, prefetch_abort_handler_addr
  LDR PC, data_abort_handler_addr
  B .
  LDR PC, irq_handler_addr
  LDR PC, fiq_handler_addr

reset_handler_addr:          .word reset_handler
undef_handler_addr:          .word undef_handler
svc_handler_addr:            .word svc_entry
prefetch_abort_handler_addr: .word prefetch_abort_handler
data_abort_handler_addr:     .word data_handler
irq_handler_addr:            .word irq_handler
fiq_handler_addr:            .word fiq_handler

vectors_end:

switchPgdir:	// switch pgdir to new PROC's pgdir; passed in r0
  // r0 contains address of PROC's pgdir address	
  mcr p15, 0, r0, c2, c0, 0   // set TTBase
  mov r1, #0
  mcr p15, 0, r1, c8, c7, 0   // flush TLB
  mcr p15, 0, r1, c7, c7, 0   // Invalidate I and D cache
  mrc p15, 0, r2, c2, c0, 0

  // set domain: all 01=client(check permission) 11=manager(no check)
  mov r0, #0x5                // APAP=0101 for CLIENT
  mcr p15, 0, r0, c3, c0, 0
  mov pc, lr                  // return

// undefined instruction below:
get_prefetch_addr:
  MRC p15, 0, r0, c6, c0, 2 // read IFAR
  mov pc, lr

get_prefetch_status:
  MRC p15, 0, r0, c5, c0, 1 // read IFSR
  mov pc, lr

get_fault_status:	// read and return MMU reg 5
  MRC p15,0,r0,c5,c0,0    // read DFSR
  mov pc, lr	
get_fault_addr:	         // read and return MMU reg 6
  MRC p15,0,r0,c6,c0,0    // read DFSR
  mov pc, lr	
get_spsr:
  mrs r0, spsr
  mov pc, lr

	.end


