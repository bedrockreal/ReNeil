#include "boutiste.h"
#define _DEFAULT_SOURCE

#include "gci.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool convertGCIBlock(GCIBlock *destGCIBlock, GCIBlock *srcGCIBlock, uint32_t *checksum, GCIFileType srcFileType) {
	assert(srcGCIBlock != NULL);
	assert(destGCIBlock != NULL);
	uint32_t xorKey = get_be32(srcGCIBlock->xorKey);
	uint32_t sig = get_be32(srcGCIBlock->sig);
	set_be32(&destGCIBlock->xorKey, xorKey);
	destGCIBlock->xorKey = srcGCIBlock->xorKey;

	for (uint32_t j = 0; j < GCI_BLOCK_BODY_SIZE; ++j) {
		destGCIBlock->data[j] = srcGCIBlock->data[j] ^ j ^ xorKey;

		uint32_t modCtr = (xorKey & 3) + 1;
		uint8_t decodedByte = (srcFileType == GCI_FILE_TYPE_ENCODED ? destGCIBlock : srcGCIBlock)->data[j];
		*checksum = (long long)(*checksum + decodedByte);
		xorKey = xorKey ^ j ^ decodedByte;
		xorKey = (long long)(xorKey >> modCtr) | ((long long)xorKey << (long long)(0x20 - modCtr));
		/* printf("%u %u %u\n", decodedByte, xorKey, modCtr); */
		/* fflush(stdout); */
	}

	*checksum ^= xorKey;

	if (srcFileType == GCI_FILE_TYPE_ENCODED) {
		set_be32(&destGCIBlock->sig, sig);
		return sig == *checksum;
	} else {
		set_be32(&destGCIBlock->sig, *checksum);
		return 1;
	}
}

bool convertGCIFile(GCIFile *dest, GCIFile *src, uint32_t initChecksum, GCIFileType srcFileType) {
	assert(dest != NULL);
	assert(src != NULL);
	memcpy(dest->header, src->header, HEADER_SIZE);
	memcpy(dest->systemBlock, src->systemBlock, GCI_BLOCK_SIZE);

	uint32_t checksum = initChecksum;
	for (int i = 0; i < GCI_NUM_BLOCKS - 1; ++i) {
		if (2 * i == GCI_NUM_BLOCKS - 1) {
			checksum = initChecksum;
		}
		if (convertGCIBlock(&dest->gameBlocks[i], &src->gameBlocks[i], &checksum, srcFileType) == 0) {
			return 0;
		}
	}

	return 1;
}

bool readGCIFile(GCIFile *data, const char *inFileName) {
	assert(data != NULL);
	assert(inFileName != NULL);

	FILE *input = fopen(inFileName, "rb");
	if (!input) {
		fprintf(stderr, "cannot open %s: %s\n", inFileName, strerror(errno));
		return 0;
	}

	int bytesRead = fread(&data->header, 1, GCI_FILE_SIZE, input);
	fclose(input);
	/* for (int i = 0; i < GCI_NUM_BLOCKS - 1; ++i) { */
	/* 	printf("%u\n", get_be32(data->gameBlocks[i].sig)); */
	/* } */
	return bytesRead == GCI_FILE_SIZE;
}

bool writeGCIFile(GCIFile *data, const char *outFileName) {
	assert(data != NULL);
	assert(outFileName != NULL);

	FILE *output = fopen(outFileName, "wb");
	if (!output) {
		fprintf(stderr, "cannot open %s: %s\n", outFileName, strerror(errno));
		return 0;
	}

	int bytesWritten = fwrite(&data->header, 1, GCI_FILE_SIZE, output);

	fclose(output);
	return bytesWritten == GCI_FILE_SIZE;
}
