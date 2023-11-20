.text
.code 32

.global reset_handler
.global vectors_start, vectors_end
.global int_off, int_on
.global lock, unlock

reset_handler:
  LDR sp, =svc_stack
  BL copy_vectors

  MSR cpsr, #0x92
  LDR sp, =irq_stack

  MSR cpsr, #0x93
  BL main
  B .

.align 4
irq_handler:
  sub	lr, lr, #4
  stmfd	sp!, {r0-r12, lr}
  bl	IRQ_chandler  
  ldmfd	sp!, {r0-r12, pc}^


// int_on()/int_off()

int_off: // sr = int_off();
  MRS r4, cpsr
  mov r0, r4
  BIC r4, r4, #0x80   // clear bit means UNMASK IRQ interrupt
  MSR cpsr, r4
  mov pc, lr          // return original SR in R0	

int_on: // int_on(SR); 
  MSR cpsr, r0        // restore original SR in R0
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

	.global unlock
A5: // write code for unlock() to unmask IRQ for CPU
unlock:
  mrs r1, cpsr
  bic r1, r1, #0x80
  msr cpsr, r1 // mask-in IRQ (bit 7 of cspr)
  mov pc, lr // exit
	
	
	.end
