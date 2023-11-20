   .global u_entry, main0, syscall, getcpsr, uexit

	.text
.code 32
// upon entry, bl main0 => r0 contains pointer to the string in ustack

u_entry:
	bl main0
// at VA=4: syscall to exit
// if main0() ever retrun: syscall to exit(0)
	bl uexit
	
// user process issues int syscall(a,b,c,d) ==> a,b,c,d are in r0-r3	
syscall:
   swi #0
   mov pc, lr

getcpsr:
   mrs r0, cpsr
   mov pc, lr

