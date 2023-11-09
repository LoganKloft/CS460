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

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

#include "string.h"
#include "uio.c"

int getpid()
{
  int pid;
  pid = syscall(0, 111, 222, 333);
  return pid;
}    
int getppid()
{ 
  return syscall(1,0,0,0);
}

int umenu()
{
  uprintf("------------------------------------------\n");
  uprintf("ps chname kfork switch wait exit fork exec\n");
  uprintf("------------------------------------------\n");
}

int ups()
{
  return syscall(2,0,0,0);
}
int uchname()
{
  char s[32];
  uprintf("input a name string : ");
  ugetline(s);
  printf("\n");
  return syscall(3,s,0,0);
}

int ukfork()
{
  int pid;
  pid = syscall(4,0,0,0);
  uprintf("kforked child = %d\n", pid);
}

int uswitch()
{
  syscall(5,0,0,0);
}

int wait(int *status)
{
  return syscall(6, status,0,0);
}

int uwait()
{
  int pid, status;

  uprintf("%d syscall to wait for ZZOMBIE child\n", getpid());
  pid = syscall(6,&status,0,0);
  uprintf("%d waited for a ZOMBIE child=%d ", getpid(), pid);
  if (pid>0)
    uprintf("status=%x %d", status, status);
  uprintf("\n");
}
int myexit(int value)
{
  syscall(7, value, 0,0);
}

int uexit()
{
  int value;
  uprintf("input an exit value : ");

  value = geti();
  printf("\n");
  syscall(7,value,0,0);
}

int ugetusp()
{
  return syscall(8,0,0,0);
}

int fork()
{
  return syscall(9,0,0,0);
}

int exec(char *line)
{
  return syscall(10, line,0,0);
}

int ufork()
{
  int pid, mypid;
  //mypid = getpid();
  pid = syscall(9,0,0,0);// can we find out the return PC here?
  if (pid>0){
    uprintf("parent %d forked a child %d\n", getpid(), pid);
  }
  if (pid==0){
    uprintf("child %d return from fork(), pid=%d\n", getpid(), pid);
  }  
  if (pid < 0)
    uprintf("%d fork failed\n", getpid());
}

int uexec()
{
  int r, mypid; char line[64];
  mypid = getpid();
  uprintf("enter a command string : ");
  ugetline(line);
  uprintf("\nline=%s\n", line);
  r = syscall(10,line,0,0);
  if (r<0)
     uprintf("%d exec failed\n", mypid);
}

int ugetc()
{
  return syscall(90,0,0,0);
}

int uputc(char c)
{
  return syscall(91,c,0,0);
}

int getPA()
{
  return syscall(92,0,0,0);
}

int argc;
char *argv[32];
 
int token(char *line)
{
  int i;
  char *cp;
  cp = line;
  argc = 0;
  
  while (*cp != 0){
       while (*cp == ' ') *cp++ = 0;        
       if (*cp != 0)
           argv[argc++] = cp;         
       while (*cp != ' ' && *cp != 0) cp++;                  
       if (*cp != 0)   
           *cp = 0;                   
       else 
            break; 
       cp++;
  }
  argv[argc] = 0;
}


int main0(int dummy, char *s)
{
  //uprintf("main0: s = %s\n", s);
  token(s);
  main(argc, argv);
}
