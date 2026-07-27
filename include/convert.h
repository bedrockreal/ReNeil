#ifndef RENEIL_CONVERT_H
#define RENEIL_CONVERT_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define HEADER_SIZE	0x40
#define NUM_BLOCKS	13
#define BLOCK_SIZE	0x2000
#define BLOCK_BODY_SIZE	0x1ff8
#define GCI_FILE_SIZE (HEADER_SIZE + NUM_BLOCKS * BLOCK_SIZE)

typedef enum {
	DECODE,
	ENCODE
} ConvertMode;

typedef struct {
	uint32_t xorKey;
	uint8_t data[BLOCK_BODY_SIZE];
	uint32_t sig;
} BlockData;

typedef struct {
	/* char *fileName; */
	/* int globalKey; */
	uint8_t header[HEADER_SIZE];
	uint8_t systemBlock[BLOCK_SIZE];
	BlockData gameBlocks[NUM_BLOCKS - 1];
} SaveFileData;

static_assert(sizeof(BlockData) == BLOCK_SIZE);
static_assert(sizeof(SaveFileData) == GCI_FILE_SIZE);

bool convertBlock(BlockData *destBlockData, BlockData *srcBlockData, uint32_t *checksum, ConvertMode mode);
bool convertFile(SaveFileData *dest, SaveFileData *src, uint32_t initChecksum, ConvertMode mode);

#endif
