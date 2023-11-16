   .global entryPoint, main0, syscall, get_cpsr
	.text
.code 32

entryPoint:	
   bl main0
   b .	
	
// Umode process issue int syscall(a,b,c,d) ==> a,b,c,d are in r0-r3	
syscall:
   stmfd sp!, {lr} // save lr in Ustack
   swi #0
   ldmfd sp!, {pc} // restore lr into PC 

get_cpsr:
   mrs r0, cpsr
   mov pc, lr

	
	
