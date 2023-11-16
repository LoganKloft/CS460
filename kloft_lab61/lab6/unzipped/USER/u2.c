typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

#include "string.c"
#include "uio.c"
#include "ucode.c"

int main(int argc, char *argv[])
{
   uprintf("running\n");
   uprintf("argc= %d\n", argc);

   for (int i = 0; i < argc; i++)
   {
      uprintf("[%d] %s\n", i, argv[i]);
   }

   printf("test memory at 0x1000\n");
   u32 *up = 0x1000;
   *up = 123;

   printf("test memory 0x80001000 VA=4KB\n");
   up = 0x80001000;
   *up = 1234;

   printf("test memory 0x80100000 VA=1MB\n");
   up = 0x80001000;
   *up = 1234;

   printf("test memory 0x80500000 VA=5MB\n");
   up = 0x80500000;
   *up = 1234;

   int pid, ppid;
   char line[64];
   u32 mode;
   int r;

   mode = get_cpsr();
   mode = mode & 0x1F;
   printf("CPU mode=%x\n", mode);

   pid = ugetpid();
   ppid = ugetppid();

   while(1){
      pid = ugetpid();
      ppid = ugetppid();

      printf("THIS IS PROCESS %d IN UMODE AT %x PARENT=%d MAIN=%x\n",
      pid, getPA(), ppid, main);
      umenu();
      printf("input a command : ");
      ugetline(line); 
      uprintf("\n"); 

      if (strcmp(line, "getpid")==0) {
         r = ugetpid();
         uprintf("pid = %d\n", r);
      }
      else if (strcmp(line, "getppid")==0) {
         r = ugetppid();
         uprintf("ppid = %d\n", r);
      }
      else if (strcmp(line, "ps")==0) {
         r = ups();
      }
      else if (strcmp(line, "chname")==0) {
         // ask for arguments
         char s[32];
         uprintf("input a name string : ");
         ugetline(s);
         printf("\n");

         r = uchname(s);
      }
      else if (strcmp(line, "switch")==0) {
         r = uswitch();
      }
      else if (strcmp(line, "fork") == 0) {
         // // ask for arguments
         // char filename[32];
         // uprintf("input a filename : ");
         // ugetline(filename);
         // printf("\n");

         // r = ufork(filename);
         r = ufork();
         printf("fork result %d\n", r);
      }
      else if (strcmp(line, "wait") == 0) {
         int status = 0;
         r = uwait(&status);
         printf("wait result : pid %d status %d\n", r, status);
      }
      else if (strcmp(line, "exit") == 0) {
         // ask for arguments
         char buf[32];
         char exitValue;
         uprintf("input an exit value : ");
         ugetline(buf);
         exitValue = atoi(buf);
         printf("\n");

         r = uexit(exitValue);
      }
      else if (strcmp(line, "sleep") == 0) {
         // ask for arguments
         char buf[32];
         char value;
         uprintf("input a sleep value : ");
         ugetline(buf);
         value = atoi(buf);
         printf("\n");

         r = usleep(value);
      }
      else if (strcmp(line, "wakeup") == 0) {
         // ask for arguments
         char buf[32];
         char value;
         uprintf("input a wakeup value : ");
         ugetline(buf);
         value = atoi(buf);
         printf("\n");

         r = uwakeup(value);
      }
      else if (strcmp(line, "exec")==0) {
         // ask for arguments
         char s[48];
         uprintf("input an exec string : ");
         ugetline(s);
         printf("\n");

         r = uexec(s);
      }
   }
}

