
ENTRY(reset_handler)
SECTIONS
{
  . = 0x80010000;
  .text : {
      ts.o
      t.o
 }

 .data : { 

      ts.o(.data) 
      t.o (.data)

  }

 .bss : { _sbegin = .; *(.bss); _send = .; }


  . = ALIGN(8);
  . = . + 0x1000; /* 4kB of svc stack memory */
  svc_stack_top = .; 
  . = . + 0x1000; /* 4kB of irq stack memory */
  irq_stack_top = .;

}

