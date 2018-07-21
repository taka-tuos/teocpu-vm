#include "teocpu-vm.h"

uint32_t teocpu_translate_address(teocpu_t *c, uint32_t vaddr)
{
	teocpu_pagedesc *pd = (teocpu_pagedesc *)&c->m[c->r[66]];
	uint32_t pagesize = teocpu_read32_unpaged(pd->pagesize);
	uint32_t p_pagelist = teocpu_read32_unpaged(pd->p_pagelist);
	
	uint32_t pageindex = vaddr / pagesize;
	uint32_t pageoffset = vaddr % pagesize;
	
	return teocpu_read32_unpaged(&c->m[p_pagelist + pageindex * 4]) + pageoffset;
}

void teocpu_push(teocpu_t *c, uint32_t d)
{
	teocpu_genpaddr32(c, c->r[65]);
	teocpu_write32(d);
	c->r[65] -= 4;
}

void teocpu_pop(teocpu_t *c, uint32_t *d)
{
	c->r[65] += 4;
	teocpu_genpaddr32(c, c->r[65]);
	*d = teocpu_read32();
}

void teocpu_nop(teocpu_t *c)
{
	
}

void teocpu_lr(teocpu_t *c)
{
	teocpu_push(c, c->r[c->m[teocpu_translate_address(c, c->r[64])]]);
	c->r[64]++;
}

void teocpu_li(teocpu_t *c)
{
	teocpu_genpaddr32(c, c->r[64]);
	teocpu_push(c, teocpu_read32());
	c->r[64] += 4;
}

void teocpu_sr(teocpu_t *c)
{
	teocpu_pop(c, &c->r[c->m[teocpu_translate_address(c, c->r[64])]]);
	c->r[64]++;
}

void teocpu_stb(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	teocpu_pop(c, &d);
	
	c->m[teocpu_translate_address(c, base + off)] = d & 0xff;
}

void teocpu_stw(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	teocpu_pop(c, &d);
	
	teocpu_genpaddr16(c, base + off);
	teocpu_write16(d & 0xffff);
}

void teocpu_std(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	teocpu_pop(c, &d);
	
	teocpu_genpaddr32(c, base + off);
	
	//printf("std:(%08x+%08x)(%08x)\n",base, off, d);
	
	if(((base + off) >> 24) == 0xff && c->cb) c->cb(base + off, d, 0);
	else teocpu_write32(d);
}

void teocpu_ldb(teocpu_t *c)
{
	uint32_t base, off;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	
	//printf("ldb:(%08x+%08x)(%08x)(%02x)\n",base, off, teocpu_translate_address(c, base + off), c->m[teocpu_translate_address(c, base + off)]);
	
	teocpu_push(c, c->m[teocpu_translate_address(c, base + off)]);
}

void teocpu_ldw(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	
	teocpu_genpaddr16(c, base + off);
	
	d = teocpu_read16();
	
	teocpu_push(c, d);
}

void teocpu_ldd(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	
	teocpu_genpaddr32(c, base + off);
	
	if(((base + off) >> 24) == 0xff && c->cb) d = c->cb(base + off, 0, 1);
	else d = teocpu_read32();
	
	teocpu_push(c, d);
}

void teocpu_add(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	//printf("add %10d + %10d (%08x + %08x) = %d(%08x)\n",a,b,a,b,a+b,a+b);
	
	teocpu_push(c, a + b);
}

void teocpu_sub(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	//printf("sub %10d - %10d (%08x - %08x) = %d(%08x)\n",a,b,a,b,a-b,a-b);
	
	teocpu_push(c, a - b);
}

void teocpu_rsub(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	teocpu_push(c, b - a);
}

void teocpu_mul(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	teocpu_push(c, a * b);
}

void teocpu_div(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	printf("div %10d / %10d = %10d...%10d\n",a,b,a,b,a/b,a%b);
	
	teocpu_push(c, a / b);
	teocpu_push(c, a % b);
}

void teocpu_muli(teocpu_t *c)
{
	uint32_t ua, ub;
	
	teocpu_pop(c, &ua);
	teocpu_pop(c, &ub);
	
	int32_t a, b, i;
	
	a = teocpu_signed(ua);
	b = teocpu_signed(ub);
	
	i = a * b;
	
	teocpu_push(c, teocpu_unsigned(i));
}

void teocpu_divi(teocpu_t *c)
{
	uint32_t ua, ub;
	
	teocpu_pop(c, &ua);
	teocpu_pop(c, &ub);
	
	int32_t a, b, i, j;
	
	a = teocpu_signed(ua);
	b = teocpu_signed(ub);
	
	i = a / b;
	j = a % b;
	
	printf("divi %10d / %10d = %10d...%10d\n",a,b,a,b,a/b,a%b);
	
	teocpu_push(c, teocpu_unsigned(i));
	teocpu_push(c, teocpu_unsigned(j));
}

void teocpu_neg(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, (~n) + 1);
}

void teocpu_abs(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, (n - (n >> 31)) ^ (0xffffffff * (n >> 31)));
}

void teocpu_sxb(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, (0x1fffffe * (n & 0x80)) | (n & 0xff));
}

void teocpu_sxw(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, (0x1fffe * (n & 0x8000)) | (n & 0xffff));
}

void teocpu_and(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	teocpu_push(c, a & b);
}

void teocpu_or(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	teocpu_push(c, a | b);
}

void teocpu_xor(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	teocpu_push(c, a ^ b);
}

void teocpu_not(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, n ^ 0xffffffff);
}

void teocpu_lls(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	teocpu_push(c, a << b);
}

void teocpu_lrs(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	teocpu_push(c, a >> b);
}

void teocpu_cmp(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	printf("cmp:%d,%d(%08x,%08x)\n",a,b,a,b);
	
	teocpu_push(c, (a > b ? 1 : 0) | (a < b ? 2 : 0) | (a == b ? 4 : 0));
}

void teocpu_cmpi(teocpu_t *c)
{
	uint32_t ua, ub;
	
	teocpu_pop(c, &ua);
	teocpu_pop(c, &ub);
	
	int32_t a, b;
	
	a = teocpu_signed(ua);
	b = teocpu_signed(ub);
	
	printf("cmpi:%d,%d(%08x,%08x)\n",a,b,a,b);
	
	teocpu_push(c, (a > b ? 1 : 0) | (a < b ? 2 : 0) | (a == b ? 4 : 0));
}

void teocpu_tst(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, (n > 0 ? 1 : 0) | (n == 0 ? 4 : 0));
}

void teocpu_tsti(teocpu_t *c)
{
	uint32_t un;
	
	teocpu_pop(c, &un);
	
	int32_t n;
	
	n = teocpu_signed(un);
	
	teocpu_push(c, (n > 0 ? 1 : 0) | (n < 0 ? 2 : 0) | (n == 0 ? 4 : 0));
}

void teocpu_ce(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, n & 4);
}

void teocpu_cg(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, n & 1);
}

void teocpu_cl(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, n & 2);
}

void teocpu_ceg(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, n & 5);
}

void teocpu_cel(teocpu_t *c)
{
	uint32_t n;
	
	teocpu_pop(c, &n);
	
	teocpu_push(c, n & 6);
}

void teocpu_b(teocpu_t *c)
{
	uint32_t a;
	
	teocpu_pop(c, &a);
	
	c->r[64] = a;
}

void teocpu_bc(teocpu_t *c)
{
	uint32_t a, f;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &f);
	
	//printf("bc:conditional %d\n",f);
	
	if(f != 0) c->r[64] = a;
}

void teocpu_c(teocpu_t *c)
{
	uint32_t a;
	
	teocpu_pop(c, &a);
	
	teocpu_push(c, c->r[64]);
	
	c->r[64] = a;
}

void teocpu_cc(teocpu_t *c)
{
	uint32_t a, f;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &f);
	
	teocpu_push(c, c->r[64]);
	
	if(f != 0) c->r[64] = a;
}

teocpu_instruction teocpu_instructions[] = {
	teocpu_nop,
	teocpu_lr,
	teocpu_li,
	teocpu_sr,
	teocpu_stb,
	teocpu_stw,
	teocpu_std,
	teocpu_ldb,
	teocpu_ldw,
	teocpu_ldd,
	teocpu_add,
	teocpu_sub,
	teocpu_rsub,
	teocpu_mul,
	teocpu_div,
	teocpu_muli,
	teocpu_divi,
	teocpu_neg,
	teocpu_abs,
	teocpu_sxb,
	teocpu_sxw,
	teocpu_and,
	teocpu_or,
	teocpu_xor,
	teocpu_not,
	teocpu_lls,
	teocpu_lrs,
	teocpu_cmp,
	teocpu_cmpi,
	teocpu_tst,
	teocpu_tsti,
	teocpu_ce,
	teocpu_cg,
	teocpu_cl,
	teocpu_ceg,
	teocpu_cel,
	teocpu_b,
	teocpu_bc,
	teocpu_c,
	teocpu_cc,
};

char *teocpu_assembly[] = {
	"0nop",
	"1lr",
	"2li",
	"1sr",
	"0stb",
	"0stw",
	"0std",
	"0ldb",
	"0ldw",
	"0ldd",
	"0add",
	"0sub",
	"0rsub",
	"0mul",
	"0div",
	"0muli",
	"0divi",
	"0neg",
	"0abs",
	"0sxb",
	"0sxw",
	"0and",
	"0or",
	"0xor",
	"0not",
	"0lls",
	"0lrs",
	"0cmp",
	"0cmpi",
	"0tst",
	"0tsti",
	"0ce",
	"0cg",
	"0cl",
	"0ceg",
	"0cel",
	"0b",
	"0bc",
	"0c",
	"0cc",
};

void teocpu_execute(teocpu_t *c)
{
	uint8_t opcode = c->m[teocpu_translate_address(c, c->r[64])];
	
	c->r[64]++;
	
	//printf("%08x(%08x):%02x(sp=%08x)\t%s\n",c->r[64]-1,teocpu_translate_address(c, c->r[64]-1),opcode,c->r[65],teocpu_assembly[opcode]+1);
	
	if(opcode < sizeof(teocpu_instructions) / sizeof(teocpu_instruction))teocpu_instructions[opcode](c);
}
