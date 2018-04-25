#include <stdint.h>

#ifndef __TEOCPU_VM__
#define __TEOCPU_VM__

typedef struct {
	uint32_t r[67];						// r0-r63, pc, sp, pd
	uint8_t *m;							// memory
	uint32_t(*cb)(uint32_t,uint32_t,uint8_t);	// 0xff000000 - 0xffffffff memory access callback (addr, data, r/~w) (32bit access only)
} teocpu_t;

typedef struct {
	uint8_t p_pagelist[4];
	uint8_t pagesize[4];
	uint8_t pagelist_len[4];
} teocpu_pagedesc;

typedef void(*teocpu_instruction)(teocpu_t *);

#define teocpu_genpaddr16(c,a) uint8_t *paddr[2]; paddr[0] = (c)->m + teocpu_translate_address(c, (a) + 0), paddr[1] = (c)->m + teocpu_translate_address(c, (a) + 1)
#define teocpu_genpaddr32(c,a) uint8_t *paddr[4]; paddr[0] = (c)->m + teocpu_translate_address(c, (a) + 0), paddr[1] = (c)->m + teocpu_translate_address(c, (a) + 1), paddr[2] = (c)->m + teocpu_translate_address(c, (a) + 2), paddr[3] = (c)->m + teocpu_translate_address(c, (a) + 3)

#define teocpu_read16() ((((uint16_t)*(paddr[1])) << 8) | ((uint16_t)*(paddr[0])))
#define teocpu_read32() ((((uint32_t)*(paddr[3])) << 24) | (((uint32_t)*(paddr[2])) << 16) | (((uint32_t)*(paddr[1])) << 8) | ((uint32_t)*(paddr[0])))

#define teocpu_write16(d) *(paddr[0]) = (d) & 0xff, *(paddr[1]) = ((d) >> 8) & 0xff
#define teocpu_write32(d) *(paddr[0]) = (d) & 0xff, *(paddr[1]) = ((d) >> 8) & 0xff, *(paddr[2]) = ((d) >> 16) & 0xff, *(paddr[3]) = ((d) >> 24) & 0xff

#define teocpu_read16_unpaged(p) ((((uint16_t)(p)[1]) << 8) | ((uint16_t)(p)[0]))
#define teocpu_read32_unpaged(p) ((((uint32_t)(p)[3]) << 24) | (((uint32_t)(p)[2]) << 16) | (((uint32_t)(p)[1]) << 8) | ((uint32_t)(p)[0]))

#define teocpu_write16_unpaged(p,d) (p)[0] = (d) & 0xff, (p)[1] = ((d) >> 8) & 0xff
#define teocpu_write32_unpaged(p,d) (p)[0] = (d) & 0xff, (p)[1] = ((d) >> 8) & 0xff, (p)[2] = ((d) >> 16) & 0xff, (p)[3] = ((d) >> 24) & 0xff

#define teocpu_signed(v) *((int32_t *)&(v))
#define teocpu_unsigned(v) *((uint32_t *)&(v))

void teocpu_execute(teocpu_t *c);

#endif
