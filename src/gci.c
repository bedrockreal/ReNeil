#define _DEFAULT_SOURCE

#include "gci.h"

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool convertBlock(GCIBlock *destGCIBlock, GCIBlock *srcGCIBlock, uint32_t *checksum, ConvertMode mode) {
	assert(srcGCIBlock != NULL);
	assert(destGCIBlock != NULL);
	destGCIBlock->xorKey = srcGCIBlock->xorKey;

	uint32_t xorKey = be32toh(destGCIBlock->xorKey);
	for (uint32_t j = 0; j < GCI_BLOCK_BODY_SIZE; ++j) {
		destGCIBlock->data[j] = srcGCIBlock->data[j] ^ j ^ xorKey;

		uint32_t modCtr = (xorKey & 3) + 1;
		uint8_t decodedByte = (mode == DECODE ? destGCIBlock : srcGCIBlock)->data[j];
		*checksum = (long long)(*checksum + decodedByte);
		xorKey = xorKey ^ j ^ decodedByte;
		xorKey = (long long)(xorKey >> modCtr) | ((long long)xorKey << (long long)(0x20 - modCtr));
		/* printf("%u %u %u\n", decodedByte, xorKey, modCtr); */
	}

	*checksum ^= xorKey;

	/* printf("%u %u\n", *checksum, sig); */
	/* fflush(stdout); */
	if (mode == DECODE) {
		destGCIBlock->sig = srcGCIBlock->sig;
		uint32_t sig = be32toh(destGCIBlock->sig);
		return sig == *checksum;
	} else {
		destGCIBlock->sig = htobe32(*checksum);
		return 1;
	}
}

bool convertFile(GCISaveFile *dest, GCISaveFile *src, uint32_t initChecksum, ConvertMode mode) {
	assert(dest != NULL);
	assert(src != NULL);
	memcpy(dest->header, src->header, HEADER_SIZE);
	memcpy(dest->systemBlock, src->systemBlock, GCI_BLOCK_SIZE);

	uint32_t checksum = initChecksum;
	for (int i = 0; i < GCI_NUM_BLOCKS - 1; ++i) {
		if (2 * i == GCI_NUM_BLOCKS - 1) {
			checksum = initChecksum;
		}
		if (convertBlock(&dest->gameBlocks[i], &src->gameBlocks[i], &checksum, mode) == 0) {
			return 0;
		}
	}

	return 1;
}

bool readGCIFile(GCISaveFile *data, char *inFileName) {
	assert(data != NULL);
	assert(inFileName != NULL);

	FILE *input = fopen(inFileName, "rb");
	if (!input) {
		fprintf(stderr, "cannot open %s: %s\n", inFileName, strerror(errno));
		return 0;
	}

	int bytesRead = fread(&data->header, 1, GCI_FILE_SIZE, input);
	/* for (int i = 0; i < GCI_NUM_BLOCKS - 1; ++i) { */
	/* 	data->gameBlocks[i].xorKey = be32toh(data->gameBlocks[i].xorKey); */
	/* 	data->gameBlocks[i].sig = be32toh(data->gameBlocks[i].sig); */
	/* } */
	fclose(input);
	return bytesRead == GCI_FILE_SIZE;
}

bool writeGCIFile(GCISaveFile *data, char *outFileName) {
	assert(data != NULL);
	assert(outFileName != NULL);

	FILE *output = fopen(outFileName, "wb");
	if (!output) {
		fprintf(stderr, "cannot open %s: %s\n", outFileName, strerror(errno));
		return 0;
	}

	/* for (int i = 0; i < GCI_NUM_BLOCKS - 1; ++i) { */
	/* 	data->gameBlocks[i].sig = htobe32(data->gameBlocks[i].sig); */
	/* 	data->gameBlocks[i].xorKey = htobe32(data->gameBlocks); */

	int bytesWritten = fwrite(&data->header, 1, GCI_FILE_SIZE, output);

	fclose(output);
	return bytesWritten == GCI_FILE_SIZE;
}
