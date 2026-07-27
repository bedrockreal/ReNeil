#define _DEFAULT_SOURCE

#include "gci.h"

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool convertGCIBlock(GCIBlock *destGCIBlock, GCIBlock *srcGCIBlock, uint32_t *checksum, GCIFileType srcFileType) {
	assert(srcGCIBlock != NULL);
	assert(destGCIBlock != NULL);
	destGCIBlock->xorKey = srcGCIBlock->xorKey;

	uint32_t xorKey = be32toh(destGCIBlock->xorKey);
	for (uint32_t j = 0; j < GCI_BLOCK_BODY_SIZE; ++j) {
		destGCIBlock->data[j] = srcGCIBlock->data[j] ^ j ^ xorKey;

		uint32_t modCtr = (xorKey & 3) + 1;
		/* uint8_t decodedByte = (mode == DECODE ? destGCIBlock : srcGCIBlock)->data[j]; */
		uint8_t decodedByte = (srcFileType == GCI_FILE_TYPE_ENCODED ? destGCIBlock : srcGCIBlock)->data[j];
		*checksum = (long long)(*checksum + decodedByte);
		xorKey = xorKey ^ j ^ decodedByte;
		xorKey = (long long)(xorKey >> modCtr) | ((long long)xorKey << (long long)(0x20 - modCtr));
		/* printf("%u %u %u\n", decodedByte, xorKey, modCtr); */
	}

	*checksum ^= xorKey;

	/* printf("%u %u\n", *checksum, sig); */
	/* fflush(stdout); */
	if (srcFileType == GCI_FILE_TYPE_ENCODED) {
	/* if (mode == DECODE) { */
		destGCIBlock->sig = srcGCIBlock->sig;
		uint32_t sig = be32toh(destGCIBlock->sig);
		return sig == *checksum;
	} else {
		destGCIBlock->sig = htobe32(*checksum);
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

bool readGCIFile(GCIFile *data, char *inFileName) {
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

bool writeGCIFile(GCIFile *data, char *outFileName) {
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

void initGCIMeta(GCIMeta *meta, const char *fileName, GCIFileType fileType, uint32_t initChecksum) {
	/* destroyGCIMeta(meta); */
	meta = malloc(sizeof(GCIMeta));
	assert(meta != NULL);
	meta->fileName = strdup(fileName);
	meta->fileType = fileType;
	meta->initChecksum = initChecksum;
	meta->file = malloc(GCI_FILE_SIZE);
	assert(meta->fileName != NULL && meta->file != NULL);
}

void destroyGCIMeta(GCIMeta *meta) {
	if (meta != NULL) {
		if (meta->fileName != NULL) {
			free(meta->fileName);
		}
		if (meta->file != NULL) {
			free(meta->file);
		}
		free(meta);
	}
}
