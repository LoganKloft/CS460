/*********** t.c file **********/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#include "vid.c"
#include "uart.c"

extern char _binary_wsu_bmp_start;

int show_bmp(char *p, int startRow, int startCol)
{ 
   int h, w, pixel, r2, i, j; 
   unsigned char r, g, b;
   char *pp;
   
   int *q = (int *)(p+14); // skip over 14 bytes file header 
   q++;                    // skip 4 bytes in image header
   w = *q;                 // width in pixels 
   h = *(q + 1);           // height in pixels
   p += 54;                // p point at pixels now
   int reduceScaleFactor = 2;    // reduce the scale of the image by factor reduceScaleFactor CHANGE 1

   r2 = 4*((3*w+3)/4);     // row size is a multiple of 4 bytes  

   for (i = startRow + h / reduceScaleFactor - 1; i >= startRow; i--) { // CHANGE 2
     pp = p;
     for (j=startCol; j < startCol + w / reduceScaleFactor; j++){ // CHANGE 3
         b = *pp; g = *(pp+1); r = *(pp+2);
	 
         pixel = (b<<16) + (g<<8) + r; // CHANGE 4
         
	 fb[i*640 + j] = pixel;
         pp += 3 * reduceScaleFactor;    // advance pp to next pixel by 6 bytes CHANGE 5
     }
     p += r2 * reduceScaleFactor; // CHANGE 6
   }
   uprintf("\nBMP image before scale change height=%d width=%d\n", h, w);
   uprintf("\nBMP image after scale change height=%d width=%d\n", h / reduceScaleFactor, w / reduceScaleFactor);
}

int color;
UART *up;

int main()
{
   char *p;

   uart_init();
   up = &uart[0];

   fbuf_init();

   p = &_binary_wsu_bmp_start;
   show_bmp(p, 0, 0); 

   while(1);   // loop here  
}
