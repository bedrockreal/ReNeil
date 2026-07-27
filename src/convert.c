#define _DEFAULT_SOURCE

#include "convert.h"

#include <assert.h>
#include <endian.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool convertBlock(BlockData *destBlockData, BlockData *srcBlockData, uint32_t *checksum, ConvertMode mode) {
	assert(srcBlockData != NULL);
	assert(destBlockData != NULL);
	destBlockData->xorKey = srcBlockData->xorKey;

	uint32_t xorKey = be32toh(destBlockData->xorKey);
	for (uint32_t j = 0; j < BLOCK_BODY_SIZE; ++j) {
		destBlockData->data[j] = srcBlockData->data[j] ^ j ^ xorKey;

		uint32_t modCtr = (xorKey & 3) + 1;
		uint8_t decodedByte = (mode == DECODE ? destBlockData : srcBlockData)->data[j];
		*checksum = (long long)(*checksum + decodedByte);
		xorKey = xorKey ^ j ^ decodedByte;
		xorKey = (long long)(xorKey >> modCtr) | ((long long)xorKey << (long long)(0x20 - modCtr));
		/* printf("%u %u %u\n", decodedByte, xorKey, modCtr); */
	}

	*checksum ^= xorKey;

	/* printf("%u %u\n", *checksum, sig); */
	/* fflush(stdout); */
	if (mode == DECODE) {
		destBlockData->sig = srcBlockData->sig;
		uint32_t sig = be32toh(destBlockData->sig);
		return sig == *checksum;
	} else {
		destBlockData->sig = htobe32(*checksum);
		return 1;
	}
}

bool convertFile(SaveFileData *dest, SaveFileData *src, uint32_t initChecksum, ConvertMode mode) {
	assert(dest != NULL);
	assert(src != NULL);
	memcpy(dest->header, src->header, HEADER_SIZE);
	memcpy(dest->systemBlock, src->systemBlock, BLOCK_SIZE);

	uint32_t checksum = initChecksum;
	for (int i = 0; i < NUM_BLOCKS - 1; ++i) {
		if (2 * i == NUM_BLOCKS - 1) {
			checksum = initChecksum;
		}
		if (convertBlock(&dest->gameBlocks[i], &src->gameBlocks[i], &checksum, mode) == 0) {
			return 0;
		}
	}

	return 1;
}
