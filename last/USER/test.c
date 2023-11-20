
#include "ucode.c"

char tline[128];

int main(int argc, char *argv[ ])
{
  int i;
  printf("this is a test\n");

  printf("argc = %d\n");
  
  for (i=0; i<argc; i++)
    printf("argv[%d] = %s\n", i, argv[i]);

}
