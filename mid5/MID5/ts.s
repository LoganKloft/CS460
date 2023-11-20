	.text
	.code 32
	
.global reset_handler
.global vectors_start, vectors_end
.global proc, procsize
.global tswitch, scheduler, running
.global lock, unlock, int_off, int_on
	
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

  stmfd	sp!, {r0-r12, lr} // save CPU registers in running proc's kstack
	
  // mask out IRQ
  MRS r4, cpsr
  ORR r4, r4, #0x80   // set I bit of CPSR
  MSR cpsr, r4

  LDR r0, =running    // r0=&running
  LDR r1, [r0,#0]     // r1->runningPROC
  str sp, [r1,#4]     // running->ksp = sp;  save sp in proc.ksp

  bl  scheduler       // set NEW running from TOP of readyQueue

  LDR r0, =running    // resume NEW running
  LDR r1, [r0,#0]     // r1->running PROC
  lDR sp, [r1,#4]     // sp = NEW running saved sp

  // mask in IRQ
  MRS r4, cpsr
  BIC r4, r4, #0x80   // clear I bit means UNMASK IRQ interrupt
  MSR cpsr, r4
 	
  ldmfd	sp!, {r0-r12, pc}  // NOTE: return to SAME SVC mode

// unlock/lock: mask in/out IRQ interrupts
unlock:
  MRS r4, cpsr
  BIC r4, r4, #0x80   // clear bit means UNMASK IRQ interrupt
  MSR cpsr, r4
  mov pc, lr	

lock: 
  MRS r4, cpsr
  ORR r4, r4, #0x80   // set bit means MASK off IRQ interrupt 
  MSR cpsr, r4
  mov pc, lr
	
// int on(int cpsr): restore original cpsr from int_off()
int_on: // restore CPSR from r0
  MSR cpsr, r0
  mov pc, lr
	
// int sr = int_off(): mask out IRQ , return original CPSR
int_off: // return CPSR in r0
  MRS r0, cpsr
  MRS r1, cpsr
  ORR r1, r1, #0x80   // set bit means MASK off IRQ interrupt 
  MSR cpsr, r1
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
