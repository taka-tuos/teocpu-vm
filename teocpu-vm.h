#include <stdint.h>

#ifndef __TEOCPU_VM__
#define __TEOCPU_VM__

typedef struct {
	uint32_t r[66];	// r0-r63, pc, sp
	uint8_t *m;		// memory
} teocpu_t;

typedef void(*teocpu_instruction)(teocpu_t *);

#define teocpu_read16(p) ((((uint16_t)(p)[1]) << 8) | ((uint16_t)(p)[0]))
#define teocpu_read32(p) ((((uint32_t)(p)[3]) << 24) | (((uint32_t)(p)[2]) << 16) | (((uint32_t)(p)[1]) << 8) | ((uint32_t)(p)[0]))

#define teocpu_write16(p,d) (p)[0] = (d) & 0xff, (p)[1] = ((d) >> 8) & 0xff
#define teocpu_write32(p,d) (p)[0] = (d) & 0xff, (p)[1] = ((d) >> 8) & 0xff, (p)[2] = ((d) >> 16) & 0xff, (p)[3] = ((d) >> 24) & 0xff

#define teocpu_signed(v) *((int32_t *)&(v))
#define teocpu_unsigned(v) *((uint32_t *)&(v))

#endif
