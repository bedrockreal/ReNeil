#ifndef RENEIL_GCI_CONVERT_H
#define RENEIL_GCI_CONVERT_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define HEADER_SIZE	0x40
#define GCI_NUM_BLOCKS	13
#define GCI_BLOCK_SIZE	0x2000
#define GCI_BLOCK_BODY_SIZE	0x1ff8
#define GCI_FILE_SIZE (HEADER_SIZE + GCI_NUM_BLOCKS * GCI_BLOCK_SIZE)

typedef enum {
	DECODE,
	ENCODE
} ConvertMode;

typedef struct {
	uint32_t xorKey;
	uint8_t data[GCI_BLOCK_BODY_SIZE];
	uint32_t sig;
} GCIBlock;

typedef struct {
	/* char *fileName; */
	/* int globalKey; */
	uint8_t header[HEADER_SIZE];
	uint8_t systemBlock[GCI_BLOCK_SIZE];
	GCIBlock gameBlocks[GCI_NUM_BLOCKS - 1];
} GCISaveFile;

static_assert(sizeof(GCIBlock) == GCI_BLOCK_SIZE);
static_assert(sizeof(GCISaveFile) == GCI_FILE_SIZE);

bool convertBlock(GCIBlock *destGCIBlock, GCIBlock *srcGCIBlock, uint32_t *checksum, ConvertMode mode);
bool convertFile(GCISaveFile *dest, GCISaveFile *src, uint32_t initChecksum, ConvertMode mode);

bool readGCIFile(GCISaveFile *data, char *inFileName);
bool writeGCIFile(GCISaveFile *data, char *outFileName);

#endif
