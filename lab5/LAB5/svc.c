
int ktswitch()
{
  tswitch();
}

int svc_handler(int a, int b, int c, int d)
{
  int r;

  // printf("svc_handler: a=%d b=%d c=%d d=%d\n",a,b,c,d);
  switch(a){
      case 0: r = kgetpid(); break;
      case 1: r = kgetppid(); break;
      case 2: r = kps(); break;
      case 3: r = kchname((char *)b); break;
      case 4: r = ktswitch(); break;

        // 5: kfork
      case 5: r = kfork((char *)b); break;
        // 6: wait
      case 6: r = kwait((int *)b); break;
        // 7: exit
      case 7: 
        if (running->pid == 1) {
          printf("P1 can't exit by command\n");
          break;
        }
        r = kexit(b); 
        break;
        // 8: sleep
      case 8: 
        if (running->pid == 1) {
          printf("P1 can't sleep by command\n");
          break;
        }
        r = ksleep(b); 
        break;
        // 9: wakeup
      case 9: r = kwakeup(b); break;
        
      case 90: r = kgetc() & 0x7F;  break;
      case 91: r = kputc(b); break;
      case 92: r = kgetPA(); break;
      default: printf("invalid syscall %d\n", a);
  }
  running->kstack[SSIZE-14] = r;

  return r; // return to to goUmode: which will replace r0 in Kstack with r
}

