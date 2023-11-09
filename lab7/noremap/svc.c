
int svc_handler(volatile int a, int b, int c, int d)
{
  int r;
  switch(a){
     case 0: r = kgetpid(); break;
     case 1: r = kgetppid(); break;
     case 2: r = kps(); break;
     case 3: r = kchname((char *)b); break;
     case 4: r = kkfork(); break;
     case 5: r = ktswitch(); break;
     case 6: r = kkwait((int *)b); break;
     case 7: r = kexit(b); break;
     case 8: r = getusp(); break;
     case 9: r = fork(); break;
     case 10: r = exec((char *)b); break;

     case 90: r = kgetc() & 0x7F;  break;
     case 91: r = kputc(b); break;
     case 92: r = (int)running->paddr - 0x80000000; break; 
     default: printf("invalid syscall %d\n", a);
  }

  running->kstack[SSIZE-14] = r;
  
  return r;
}

