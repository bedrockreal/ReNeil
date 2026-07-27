#ifndef RENEIL_GCI_H
#define RENEIL_GCI_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define HEADER_SIZE	0x40
#define GCI_NUM_BLOCKS	13
#define GCI_BLOCK_SIZE	0x2000
#define GCI_BLOCK_BODY_SIZE	0x1ff8
#define GCI_FILE_SIZE (HEADER_SIZE + GCI_NUM_BLOCKS * GCI_BLOCK_SIZE)

typedef enum {
	GCI_FILE_TYPE_DECODED,
	GCI_FILE_TYPE_ENCODED
} GCIFileType;

typedef struct {
	uint32_t xorKey;
	uint8_t data[GCI_BLOCK_BODY_SIZE];
	uint32_t sig;
} GCIBlock;

typedef struct {
	uint8_t header[HEADER_SIZE];
	uint8_t systemBlock[GCI_BLOCK_SIZE];
	GCIBlock gameBlocks[GCI_NUM_BLOCKS - 1];
} GCIFile;

static_assert(sizeof(GCIBlock) == GCI_BLOCK_SIZE);
static_assert(sizeof(GCIFile) == GCI_FILE_SIZE);

typedef struct {
	char *fileName;
	GCIFileType fileType;
	uint32_t initChecksum;
	GCIFile *file;
} GCIMeta;

bool convertGCIBlock(GCIBlock *destGCIBlock, GCIBlock *srcGCIBlock, uint32_t *checksum, GCIFileType srcFileType);
bool convertGCIFile(GCIFile *dest, GCIFile *src, uint32_t initChecksum, GCIFileType srcFileType);

bool readGCIFile(GCIFile *data, char *inFileName);
bool writeGCIFile(GCIFile *data, char *outFileName);

void initGCIMeta(GCIMeta *meta, const char *fileName, GCIFileType fileType, uint32_t initChecksum);
void destroyGCIMeta(GCIMeta *meta);

#endif
