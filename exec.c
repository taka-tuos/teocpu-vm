#include "teocpu-vm.h"
#include <stdlib.h>
#include <stdio.h>

uint32_t callback_syscall(uint32_t a, uint32_t d, uint8_t rw)
{
	//printf("R/W REQ(%s)\n", rw ? "R" : "W");
	if(!rw) printf("%c", d);
	if(!rw && d == 0x0a) exit(0);
	return 0;
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		puts("usage>exec-teocpu <binary>");
		return 1;
	}
	
	teocpu_t c;
	c.m = malloc(1024*1024);
	c.cb = callback_syscall;
	c.r[64] = 0;
	c.r[65] = 0x1ffff;
	
	FILE *fp = fopen(argv[1],"rb");
	
	fread(c.m,1,65536,fp);
	
	fclose(fp);
	
	while(1) teocpu_execute(&c);
	
	return 0;
}