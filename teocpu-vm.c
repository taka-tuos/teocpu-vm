#include "teoccpu-vm.h"

void teocpu_push(teocpu_t *c, uint32_t d)
{
	teocpu_write32(c->m + c->r[65], d);
	c->r[65] += 4;
}

void teocpu_pop(teocpu_t *c, uint32_t *d)
{
	c->r[65] -= 4;
	*d = teocpu_read32(c->m + c->r[65]);
}

void teocpu_nop(teocpu_t *c)
{
	
}

void teocpu_lr(teocpu_t *c)
{
	teocpu_push(c, c->r[c->m[c->r[64]]]);
	c->r[64]++;
}

void teocpu_li(teocpu_t *c)
{
	teocpu_push(c, teocpu_read32(c->m + c->r[64]);
	c->r[64] += 4;
}

void teocpu_sr(teocpu_t *c)
{
	teocpu_pop(c, &c->r[c->m[c->r[64]]]);
	c->r[64]++;
}

void teocpu_stb(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	teocpu_pop(c, &d);
	
	c->m[base + off] = d & 0xff;
}

void teocpu_stw(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	teocpu_pop(c, &d);
	
	teocpu_write16(c->m + base + off, d & 0xffff);
}

void teocpu_std(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	teocpu_pop(c, &d);
	
	teocpu_write32(c->m + base + off, d & 0xffff);
}

void teocpu_ldb(teocpu_t *c)
{
	uint32_t base, off;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	
	teocpu_push(c, c->m[base + off]);
}

void teocpu_ldw(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	
	d = teocpu_read16(c->m + base + off);
	
	teocpu_push(c, d);
}

void teocpu_ldd(teocpu_t *c)
{
	uint32_t base, off, d;
	
	teocpu_pop(c, &off);
	teocpu_pop(c, &base);
	
	d = teocpu_read32(c->m + base + off);
	
	teocpu_push(c, d);
}

void teocpu_add(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	teocpu_push(c, a + b);
}

void teocpu_sub(teocpu_t *c)
{
	uint32_t a, b;
	
	teocpu_pop(c, &a);
	teocpu_pop(c, &b);
	
	teocpu_push(c, a - b);
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
	teocpu_mul,
	teocpu_div,
	teocpu_muli,
	teocpu_divi,
	teocpu_neg,
	teocpu_abs,
	teocpu_sxb,
	teocpu_sxw,
};

void teocpu_execute(teocpu_t *c)
{
	uint8_t opcode = c->m[c->r[64]];
	
	c->r[64]++;
	
	teocpu_instructions[opcode](c);
}
