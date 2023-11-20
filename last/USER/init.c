// simple init.c as in BOOK

#include "ucode.c"

int console;
        
int parent()
{
  int pid, status;

  while(1){
    printf("KCINIT : waiting for ZOMBIE child\n");
    pid = wait(&status);

    if (pid==console){
      printf("INIT fork a new console login\n");
       console = fork();
       if (console)
	 continue;
       else
	 exec("login /dev/tty0");
    }
    printf("KCINIT: I just buried an orphan child task %d\n", pid);
  }
}

int main(int argc, char *argv[ ])
{
  int in, out;
  in  = open("/dev/tty0", O_RDONLY);
  out = open("/dev/tty0", O_WRONLY);

  printf("KCW's SIMPLE INIT : fork a login proc on console\n");

  console = fork();
  if (console)      // parent
    parent();
  else
    exec("login /dev/tty0");
}

