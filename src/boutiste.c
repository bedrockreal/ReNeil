#define _DEFAULT_SOURCE

#include "boutiste.h"
#include <endian.h>

uint16_t get_be16(uint16_be be16) {
	return be16toh(be16._u16);
}

void set_be16(uint16_be *be16, uint16_t value) {
	be16->_u16 = htobe16(value);
}

uint32_t get_be32(uint32_be be32) {
	return be32toh(be32._u32);
}

void set_be32(uint32_be *be32, uint32_t value) {
	be32->_u32 = htobe32(value);
}
