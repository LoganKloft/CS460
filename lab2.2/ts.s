/*********** ts.s file **********/
	
.text
.code 32

	.global start

start:	
  LDR sp, =stack_top   // set SVC mode stack pointer
  BL main
  B .
	
