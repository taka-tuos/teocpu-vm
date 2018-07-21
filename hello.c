void putchar(int c)
{
	asm("\tpi($8)\n");
	asm("\tpr(%d34)\n");
	asm("\tpr(%d24)\n");
	asm("\tldd()\n");
	asm("\tpr(%d24)\n");
	asm("\ttx()\n");
}

void puts(char *s)
{
	for(;*s;s++) putchar(*s);
}

void main(void)
{
	puts("hello,world\n");
	while(1);
}
