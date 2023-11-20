
ENTRY(reset_handler)
SECTIONS
{
  . = 0x10000;
  .text : {
      ts.o
      t.o
 }
 .data : { 
      ts.o(.data) 
      t.o (.data)
  }
 .bss : { *(.bss) }

  . = ALIGN(8);
  . = . + 0x1000; /* 4kB of svc stack memory */
  svc_stack_top = .;
  . = . + 0x1000; /* 4kB of irq stack memory */
  irq_stack_top = .;
  . = . + 0x1000; /* 4kB of abt stack memory */
  abt_stack_top = .;
  . = . + 0x1000; /* 4kB of und stack memory */
  und_stack_top = .;

}

