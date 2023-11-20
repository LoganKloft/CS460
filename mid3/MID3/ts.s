// ts.s file
	
	.text
.code 32
.global reset_handler
.global vectors_start, vectors_end
.global proc, procsize
.global tswitch, scheduler, running, gosvc, goirq, mytswitch
.global int_off, int_on, lock, unlock
	
reset_handler:
  LDR sp, =svc_stack_top
	
  BL copy_vectors

  MSR cpsr, #0x12
  LDR sp, =irq_stack_top

  MSR cpsr, #0x13
  BL main
  B .

.align 4
irq_handler: // when irq_handler exits, I want to return to tswitch if timer = 0
             // otherwise, I want to do nothing. This means I must set pc to tswitch 
             // BUT, r0-r12 must remain the same
  LDR   sp, =irq_stack_top
  sub	lr, lr, #4
  stmfd	sp!, {r0-r12, lr}

  bl	IRQ_handler  

  // if r0 == 0, then continue as normal
  cmp r0, #0
  bne irq_handler_tswitch
  ldmfd	sp!, {r0-r12, pc}^

irq_handler_tswitch:
  // if r0 == 1, then we execute tswitch

  // START: SET UP MYTSWITCH (exactly like the start of tswitch)
  // 1: STORE IRQ FRAME IN SVC STACK
  // pop from stack
  ldmfd sp!, {r0-r12, lr}

  // go to svc
  msr cpsr, #0x93

  // push r0-r12 to svc
  stmfd sp!, {r0-r12}

  // back to irq
  msr cpsr, #0x92

  // store lr in r0
  mov r0, lr

  // back to svc
  msr cpsr, #0x93

  // store r0 in lr
  mov lr, r0

  // pop r0-12
  ldmfd sp!, {r0-r12}

  // push r0-r12 and lr
  stmfd	sp!, {r0-r12, lr}

  // 2: UPDATE RUNNING->KSP
  // set running->ksp to sp
  LDR r0, =running // r0=&running
  LDR r1, [r0,#0]  // r1->runningPROC
  str sp, [r1,#4]  // running->ksp = sp
  // END: SET UP MYTSWITCH

  // WE CAN SWITCH TO NEXT PROCESS NOW THAT FRAME AND KSP ARE SET UP
  // enable interrupts
  msr cpsr, #13

  // call mytswitch
  ldr pc, =mytswitch

mytswitch:

  // skip setup
  // we already pushed r0-r12 and r14 to the stack
  // we already stored SVC.sp in running->ksp

  bl scheduler

  LDR r0, =running
  LDR r1, [r0,#0]     // r1->runningPROC
  lDR sp, [r1,#4]
	
  ldmfd	sp!, {r0-r12, pc}

tswitch:
  // push r0-r12 and r14 to the stack
  stmfd	sp!, {r0-r12, lr}

  // set running->ksp = sp
  LDR r0, =running // r0=&running
  LDR r1, [r0,#0]  // r1->runningPROC
  str sp, [r1,#4]  // running->ksp = sp

  bl	scheduler

  LDR r0, =running
  LDR r1, [r0,#0]     // r1->runningPROC
  lDR sp, [r1,#4]
	
  ldmfd	sp!, {r0-r12, pc}

int_on: //  original cpsr in r0
  MSR cpsr, r0
  mov pc, lr	

int_off: // return original CPSR
  MRS r4, cpsr
  ORR r4, r4, #0x80   // set bit means MASK off IRQ interrupt 
  MSR cpsr, r4
  mov r0, r4
  mov pc, lr	
	
lock:
  MRS r4, cpsr
  ORR r4, r4, #0x80   // set bit means MASK off IRQ interrupt 
  MSR cpsr, r4
  mov pc, lr	

unlock:	
  MRS r0, cpsr
  BIC r0, r0, #0x80   // clear bit means UNMASK IRQ interrupt
  MSR cpsr, r0
  mov pc, lr	

	.global getcpsr
getcpsr:
	MRS r0, cpsr
	mov pc, lr

vectors_start:
  LDR PC, reset_handler_addr
  LDR PC, undef_handler_addr
  LDR PC, swi_handler_addr
  LDR PC, prefetch_abort_handler_addr
  LDR PC, data_abort_handler_addr
  B .
  LDR PC, irq_handler_addr
  LDR PC, fiq_handler_addr

reset_handler_addr:          .word reset_handler
undef_handler_addr:          .word undef_handler
swi_handler_addr:            .word swi_handler
prefetch_abort_handler_addr: .word prefetch_abort_handler
data_abort_handler_addr:     .word data_abort_handler
irq_handler_addr:            .word irq_handler
fiq_handler_addr:            .word fiq_handler

vectors_end:

	.end
