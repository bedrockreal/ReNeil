#include <stdint.h>
#define _DEFAULT_SOURCE

#ifdef __cplusplus
extern "C" {
#endif
#include "convert.h"

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
}
#endif

bool readGCIFile(SaveFileData *data, char *inFileName) {
	assert(data != NULL);
	assert(inFileName != NULL);

	FILE *input = fopen(inFileName, "rb");
	if (!input) {
		fprintf(stderr, "cannot open %s: %s\n", inFileName, strerror(errno));
		return 0;
	}

	int bytesRead = fread(&data->header, 1, GCI_FILE_SIZE, input);
	/* for (int i = 0; i < NUM_BLOCKS - 1; ++i) { */
	/* 	data->gameBlocks[i].xorKey = be32toh(data->gameBlocks[i].xorKey); */
	/* 	data->gameBlocks[i].sig = be32toh(data->gameBlocks[i].sig); */
	/* } */
	fclose(input);
	return bytesRead == GCI_FILE_SIZE;
}

bool writeGCIFile(SaveFileData *data, char *outFileName) {
	assert(data != NULL);
	assert(outFileName != NULL);

	FILE *output = fopen(outFileName, "wb");
	if (!output) {
		fprintf(stderr, "cannot open %s: %s\n", outFileName, strerror(errno));
		return 0;
	}

	/* for (int i = 0; i < NUM_BLOCKS - 1; ++i) { */
	/* 	data->gameBlocks[i].sig = htobe32(data->gameBlocks[i].sig); */
	/* 	data->gameBlocks[i].xorKey = htobe32(data->gameBlocks); */

	int bytesWritten = fwrite(&data->header, 1, GCI_FILE_SIZE, output);

	fclose(output);
	return bytesWritten == GCI_FILE_SIZE;
}

void handleInvalidArgs(char *programName) {
	fprintf(stderr, "Usage: %s d|e <infile> <outfile> [init_checksum]\n", programName);
	exit(1);
}

int main(int argc, char **argv) {
	if (argc < 4 || argc > 5) {
		handleInvalidArgs(argv[0]);
	}

	uint32_t initChecksum = 0x12345678;
	if (argc == 5) {
		initChecksum = (uint32_t)strtol(argv[4], NULL, 16);
	}

	ConvertMode mode;
	if (strcmp(argv[1], "d") == 0) {
		mode = DECODE;
	} else if (strcmp(argv[1], "e") == 0) {
		mode = ENCODE;
	} else {
		handleInvalidArgs(argv[0]);
	}

	SaveFileData srcData, destData;
	assert(readGCIFile(&srcData, argv[2]));
	assert(convertFile(&destData, &srcData, initChecksum, mode));
	assert(writeGCIFile(&destData, argv[3]));
}
