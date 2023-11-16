
ENTRY(u_entry)

SECTIONS
{
  . = 0x0;
  .text : {
     us.o
     *(.text) 
  }
  .data : { *(.data) }
  .bss  : { *(.bss)  }
}

