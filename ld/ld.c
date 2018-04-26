#include "../teocpu-vm.h"

#define READ32(p) teocpu_read32_unpaged(p)
#define WRITE32(p,d) teocpu_write32_unpaged(p,d)
#define SIGN "TEOC"

#include "kagura_standard_ld.c"
