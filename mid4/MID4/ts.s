
	.text
.code 32
.global reset_handler
.global vectors_start, vectors_end
.global proc, procsize
.global tswitch, scheduler, running
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
irq_handler:
  LDR   sp, =irq_stack_top
  sub	lr, lr, #4
  stmfd	sp!, {r0-r12, lr}
  bl	IRQ_handler  
  ldmfd	sp!, {r0-r12, pc}^

tswitch:
// disable IRQ interrupts
  MRS r0, cpsr
  ORR r0, r0, #0x80   // set bit means MASK off IRQ interrupt 
  MSR cpsr, r0
	
  stmfd	sp!, {r0-r12, lr}

  LDR r0, =running // r0=&running
  LDR r1, [r0,#0]  // r1->runningPROC
  str sp, [r1,#4]  // running->ksp = sp

  bl	scheduler

  LDR r0, =running
  LDR r1, [r0,#0]     // r1->runningPROC
  lDR sp, [r1,#4]
	
  // enable IRQ interrupts
  MRS r0, cpsr
  BIC r0, r0, #0x80   // clear bit means UNMASK IRQ interrupt
  MSR cpsr, r0

  ldmfd	sp!, {r0-r12, pc}

// int_on()/int_off(): turn on/off IRQ interrupts
int_on: //  parameter in r0
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
/*************************
	.global getsp

getsp:	mov r0, sp
	mov pc, lr
	
	.global getvsp
getvsp:	
        mrs r4, cpsr	
	MSR cpsr, #0x13
	mov r0, sp
	MSR cpsr, r4
	mov pc, lr

	.global getisp
getisp:
	mrs r4, cpsr
	MSR cpsr, #0x12
	mov r0, sp
	MSR cpsr, r4
	mov pc, lr
**********************/
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
