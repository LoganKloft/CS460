int umenu()
{
  uprintf("------------------------------------\n");
  uprintf("ps chname switch wait exit fork exec\n");
  uprintf("------------------------------------\n");
}

int ugetpid()
{
  return syscall(0,0,0,0);
}    

int ugetppid()
{ 
  return syscall(1,0,0,0);
}

int ups()
{
  return syscall(2,0,0,0);
}

int uchname(char *s)
{
  return syscall(3,s,0,0);
}

int uswitch()
{
  return syscall(4,0,0,0);
}

// int ufork(char *filename)
// {
//   return syscall(5,filename,0,0);
// }

int ufork()
{
  return syscall(5,0,0,0);
}

int uwait(int *status)
{
  return syscall(6,status,0,0);
}

int uexit(int exitValue)
{
  return syscall(7,exitValue,0,0);
}

int usleep(int value)
{
  return syscall(8, value,0,0);
}

int uwakeup(int value)
{
  return syscall(9, value,0,0);
}

int uexec(char *s)
{
  return syscall(10,s,0,0);
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