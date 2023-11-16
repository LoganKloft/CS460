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

// interrupts.c file

int kprintf(char *fmt, ...);

/* all other handlers are infinite loops */

void __attribute__((interrupt)) undef_handler(void)
{
  kprintf("undef exception\n");
  while(1);
}

//void __attribute__((interrupt)) swi_handler(void) { for(;;); }
// https://developer.arm.com/documentation/ddi0595/2021-12/AArch32-Registers/IFSR--Instruction-Fault-Status-Register
// https://developer.arm.com/documentation/ddi0406/cb/System-Level-Architecture/Protected-Memory-System-Architecture--PMSA-/Exception-reporting-in-a-PMSA-implementation/Fault-Status-Register-encodings-for-the-PMSA?lang=en#CBHCIDHC 
void __attribute__((interrupt)) prefetch_abort_handler(void) 
{ 
  u32 prefetch_status = get_prefetch_status();
  // u32 prefetch_address = get_prefetch_addr();
  // kprintf("prefetch exception at %x\n", prefetch_address);

  // status is the 10th bit + 3:0 bits
  u32 status = prefetch_status & 0b1000001111;
  if (status == 0b00001)
  {
    kprintf("Alignment fault\n");
  }
  else if (status == 0b00000)
  {
    kprintf("Background fault\n");
  }
  else if (status == 0b01101)
  {
    kprintf("Permission Fault\n");
  }
  else if (status == 0b00010)
  {
    kprintf("Debug event that generates a Prefetch Abort exception\n");
  }
  else if (status == 0b01000)
  {
    kprintf("Synchronous external abort\n");
  }
  else if (status == 0b10100)
  {
    kprintf("implementation defined\n");
  }
  else if (status == 0b11010)
  {
    kprintf("implementation defined\n");
  }
  else if (status == 0b11001)
  {
    kprintf("Memory access synchronous parity error\n");
  }

  while(1);
}


void __attribute__((interrupt)) fiq_handler(void) { for(;;); }


