/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

   .global entryPoint, main0, syscall, getmysp, getcsr, getmyaddress
	.text
.code 32
// upon entry, bl main0 => r0 contains pointer to the string in ustack
	
//. = 0x40
entryPoint:	
	bl main0
	
// if main0() ever retrun: syscall to exit(0)
	
@ user process issues int syscall(a,b,c,d) ==> a,b,c,d are in r0-r3	
syscall:
   stmfd sp!, {lr}
	swi #0
   ldmfd sp!, {lr}
   mov pc, lr

getmysp:
   mov r0, sp
   mov pc, lr

getcsr:
   mrs r0, cpsr
   mov pc, lr
getmyaddress:
   ldr r0, =main0
   mov pc, lr
	
	
