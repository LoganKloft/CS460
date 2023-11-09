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

#include "ucode.c"
extern int entryPoint;

int main(int argc, char *argv[])
{
  int i, a,b,c, pid, ppid, mode, r;
  u32 usp, usp1;
  char line[64]; char uc;

  mode = getcsr();
  mode = mode & (0x0000001F);
  printf("CPU mode=%x argc=%d\n", mode, argc);

  for (i=0; i<argc; i++){
    printf("argv[%d]=%s\n", i, argv[i]);
  }
  //printf("\n");

  while(1){
    pid  = getpid();
    ppid = getppid();
    printf("This is process #%d in Umode at VA=%x PA=%x parent=%d\n", 
	   pid, &entryPoint, getPA(), ppid);
 
    umenu();
    printf("input a command : ");
    ugetline(line); 
    uprintf("\n"); 
 
    if (strcmp(line, "ps")==0)
      ups();
    if (strcmp(line, "chname")==0)
       uchname();
    if (strcmp(line, "kfork")==0)
       ukfork();
    if (strcmp(line, "switch")==0)
       uswitch();
    if (strcmp(line, "exit")==0)
       uexit();

    if (strcmp(line, "wait")==0)
       uwait();
    if (strcmp(line, "fork")==0)
       ufork();
    if (strcmp(line, "exec")==0)
       uexec();

  }
}
