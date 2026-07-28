#ifndef RENEIL_ENDIAN_H
#define RENEIL_ENDIAN_H

#include <stdint.h>

typedef union {
	uint16_t _u16;
	uint8_t bytes[2];
} uint16_be;

typedef union {
	uint32_t _u32;
	uint8_t bytes[4];
} uint32_be;

uint16_t get_be16(uint16_be be16);
void set_be16(uint16_be *be16, uint16_t value);

uint32_t get_be32(uint32_be be32);
void set_be32(uint32_be *be32, uint32_t value);

#endif
