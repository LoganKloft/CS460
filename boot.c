int boot()
{ 
  char   *name[2];  // name[0]="boot";   name[1]="eos";
  char   *address = 0x100000;  // EOS kernel loading address     
  uprints("in boot\n\r");
  
  /*****************************
   sdimage is a disk image containing an EXT2 FS
   EOS kernel image is the file /boot/eos

   t.c:      booter();

             -------- YOUR task is to these ------- 
   booter(): FIND the file /boot/eos
             load its data blocks to 1MB = 0x100000
             return 1 to caller
             -------------------------------------
   caller:   call go() in ts.s:
             mov pc, 0x100000

  ****************************/
}  

