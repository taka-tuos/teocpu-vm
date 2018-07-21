#include "xprintf.h"

void putchar(unsigned char c)
{
	*((unsigned int *)0xff000000) = c;
}

void main(void)
{
	int n = 114514;
	
	xdev_out(putchar);
	
	
	
	/*if(n > 0) xputs("n > 0");
	if(n < 0) xputs("n < 0");*/
	xprintf("The C++%d TOO LATE Programming Language\n",114514);
	while(1);
}
