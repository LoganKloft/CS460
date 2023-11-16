/***********************************************************************
                      uio.c file of WANIX
***********************************************************************/
char space = ' ';
char *ctable = "0123456789ABCDEF";
char cr = '\r';

int mputc(char c)
{
  char cc;
  cc = c;
  write(1, &cc, 1);
   if (c=='\n')
     write(1,&cr,1);
   return 0;
}


int puts(const char *s)
{
  char line[128];
  strcpy(line, s);
  strcat(line, "\n\r");
  write(1, line, strlen(line));
}
int putchar(char c)
{
  putc(c);
}

int putc(char c)
{
  write(1, &c, 1);
}

#define printk printf

int printf(char *fmt,...);
#define EOF -1
extern char cr;

int getc()
{
   int c, n;
   n = read(0, &c, 1);

   /********************************************************************* 
   getc from KBD will NOT get 0 byte but reading file (after redirect 0 
   to file) may get 0 byte ==> MUST return 2-byte -1 to differentiate.
   **********************************************************************/

   if (n==0 || c==4 || c==0 ) return EOF;  
                                
   return (c&0x7F);
}

// getline() does NOT show the input chars AND no cooking: 
// for reditected inputs from a file which may contain '\b' chars

int getline(char *s)
{
  int c;  
  char *cp = s;
  
  c = getc();

  while ((c != EOF) && (c != '\r') && (c != '\n')){
    *cp++ = c;
     c = getc();
  }
  if (c==EOF) return 0;

  *cp++ = c;         // a string with last char=\n or \r
  *cp = 0;    
  //printf("getline: %s", s); 
  return strlen(s);  // at least 1 because last char=\r or \n
}


// gets() show each input char AND cook input line

int gets(char *s)
{
  int c;
  char *cp, *cq, temp[128];
  
  cp = temp;    // get chars into temp[] first

  c = getc();
  while (c!= EOF && c != '\r' && c != '\n'){
    *cp++ = c;
    mputc(c);
    if (c == '\b'){ // handle \b key
      mputc(' ');
      mputc('\b');
    }
    c = getc();
  }
  mputc('\n'); mputc('\r');

  if (c==EOF) return 0;
  
  *cp = 0;   

  // printf("temp=%s\n", temp);

  // cook line in temp[] into s
  cp = temp; cq = s; 

  while (*cp){
    if (*cp == '\b'){
      if (cq > s)
	  cq--; 
      cp++;
      continue;
    }
    *cq++ = *cp++;
  }
  *cq = 0;

  //printf("s=%s\n", s);

  return strlen(s)+1;  // line=CR or \n only return 1
}


void prints(char *s)
{
   while (*s){
      mputc(*s);
      s++;
   }
}

void mputs(char *s)
{
  prints(s);
}


extern int strlen(const char *);
void print2f(char *s)
{
  write(2, s, (int)strlen(s));
}

void rpi(int x)
{
   char c;
   if (x==0) return;
   c = ctable[x%10];
   rpi((int)x/10);
   mputc(c);
}
  
void printi(int x)
{
    if (x==0){
       prints("0 ");
       return;
    }
    if (x < 0){
       mputc('-');
       x = -x;
    }
    rpi((int)x);
    mputc(space);
}

void rpu(u32 x)
{
   char c;
   if (x==0) return;
   c = ctable[x%10];
   rpi((u32)x/10);
   mputc(c);
}

void printu(u32 x)
{
    if (x==0){
       prints("0 ");
       return;
    }
    rpu((u32)x);
    mputc(space);
}

void rpx(u32 x)
{
   char c;
   if (x==0) return;
   c = ctable[x%16];
   rpx((u32)x/16);
   mputc(c);
}

void printx(u32 x)
{  
  prints("0x");
   if (x==0){
      prints("0 ");
      return;
   }
   rpx((u32)x);
  mputc(space);
}


void printc(char c)
{
  mputc(c);
  c = c&0x7F;
  if (c=='\n')
    mputc(cr);
}

int printk(char *fmt,...)
{
  char *cp, *cq;
  int  *ip;

  cq = cp = (char *)fmt;
  ip = (int *)&fmt + 1;

  while (*cp){
    if (*cp != '%'){
       printc(*cp);
       cp++;
       continue;
    }
    cp++;
    switch(*cp){
      case 'd' : printi(*ip); break;
      case 'u' : printu(*ip); break;
      case 'x' : printx(*ip); break;
      case 's' : prints((char *)*ip); break;
      case 'c' : printc((char)*ip);   break;
    }
    cp++; ip++;
  }
}
