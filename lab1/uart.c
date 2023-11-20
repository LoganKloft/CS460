/************** uart.c file *******************
Flag register (offset 0x18): status of UART port

   7    6    5    4    3   2   1   0
| TXFE RXFF TXFF RXFE BUSY -   -   - |
  
TXFE=Tx buffer empty
RXFF=Rx buffer full
TXFF=Tx buffer full
RXFE=Rx buffer empty
BUSY = device busy
**********************************************/
  
#define DR 0x00
#define FR 0x18

typedef struct uart{
  char *base;           // base address
  int  n;               // UART number 
}UART;

UART *up;

// versatile_epb : UART0 at 0x101F1000

char *ctable = "0123456789ABCDEF";

void uart_init()
{
  up->base = (UART *)(0x101F1000);
  up->n = 0;
}

void uputc(char c)
{
  while ( *(up->base + FR) & 0x20 ); // while FR.bit5 != 0 ;  (TX FULL)
  *(up->base + DR) = (int)c;         // write c to DR
}

int ugetc()
{
  while ( *(up->base + FR) & 0x10 ); // while FR.bit4 != 0 ;  (RX EMPTY)
  return *(up->base + DR);           // get data in DR
}

int uprints(char *s)
{
  while(*s)
    uputc(*s++);
}

// get a string from the user
int ugets(char s[])
{

}

int urpu(u32 x)
{  
  char c;
  if (x){
    c = ctable[x % 10];
    urpu(x / 10);
    uputc(c);
  }
}

int rpu(u32 x)
{  
  char c;
  if (x){
    c = ctable[x % 16];
    urpu(x / 16);
    uputc(c);
  }
}

int uprintu(u32 x)
{
  (x==0)? uputc('0') : urpu(x);
}

int uprintx(u32 x)
{
  uputc('0');
  uputc('x');
  (x==0)? uputc('0') : rpu(x);
}

int uprintd(int x)
{
  if (x < 0)
  {
    uputc('-');
    x = -x;
  }

  (x==0)? uputc('0') : urpu(x);
}

int uprintf(char *fmt, ...)
{
  int *ip = &fmt;
  ip++;

  while (*fmt)
  {
    char c = *fmt;
    switch(c)
    {
      case '%':
        fmt++;
        c = *fmt;
        switch(c)
        {
            case 'c':
                uputc(*ip);
                break;
            case 's':
                uprints(*ip);
                break;
            case 'u':
                uprintu(*ip);
                break;
            case 'x':
                uprintx(*ip);
                break;
            case 'd':
                uprintd(*ip);
                break;
        }
        ip++;
        break;
      case '\n':
        uputc('\n');
        uputc('\r');
        break;
      default:
        uputc(c);
    }
    fmt++;
  }
}