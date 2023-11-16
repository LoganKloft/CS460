/******* test.c file *********/

#include "ucode.c"

int main(int argc, char *argv[ ])
{
  int i;
  printf("this is KCW's NEW test\n");
  
  printf("print argc, argv\n");

  printf("argc=%d\n", argc);
  
  for (i=0; i<argc; i++)
    printf("argv[%d] = %s\n", i, argv[i]);

  // optional: exit(0);
}
