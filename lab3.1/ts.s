
.text
.code 32

.global reset_handler
.global vectors_start, vectors_end

reset_handler:
  LDR sp, =svc_stack     // set SVC stack
  BL copy_vectors        // copy vector table to 0 

  MSR cpsr, #0x92        // to IRQ mode 0x92=b1001 0010 : Ibit=1, mode=10010
  LDR sp, =irq_stack     // set IRQ stack

  MSR cpsr, #0x93        // back to SVC mode with I-bit=1: masked out IRQ
  BL main                // call main
  B .

.align 4
irq_handler:             // IRQ interrupt handler
  sub	lr, lr, #4
  stmfd	sp!, {r0-r12, lr}
  bl	IRQ_chandler  
  ldmfd	sp!, {r0-r12, pc}^
	
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
A4: // write code for unlock() to unmask IRQ
unlock:	
  mrs r1, cpsr
  bic r1, r1, #0x80
  msr cpsr, r1 // mask-in IRQ (bit 7 of cspr)
  mov pc, lr // exit



.end
