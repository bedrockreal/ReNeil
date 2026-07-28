#include "gci_pair.hpp"
#include <cassert>
#include <cstdlib>

extern "C" {
#include "gci.h"
}

GCIPair::GCIPair() {}

GCIPair::GCIPair(std::string encodedFileName, uint32_t initChecksum) {
	encodedFile = nullptr;
	decodedFile = nullptr;
	init(encodedFileName, initChecksum);
}

GCIPair::~GCIPair() {
	free(encodedFile);
	free(decodedFile);
	encodedFile = nullptr;
	decodedFile = nullptr;
}

void GCIPair::init(std::string encodedFileName, uint32_t initChecksum) {
	this->initChecksum = initChecksum;
	this->encodedFileName = encodedFileName;
	if (encodedFile == nullptr) {
		encodedFile = static_cast<GCIFile *>(malloc(sizeof(GCIFile)));
	}
	if (decodedFile == nullptr) {
		decodedFile = static_cast<GCIFile *>(malloc(sizeof(GCIFile)));
	}

	if (readGCIFile(encodedFile, encodedFileName.c_str()) == 0) {
		initSuccess = 0;
	}
	if (convertGCIFile(decodedFile, encodedFile, initChecksum, GCI_FILE_TYPE_ENCODED) == 0) {
		initSuccess = 0;
	}
	initSuccess = 1;
}

bool GCIPair::isInitSuccess() {
	return initSuccess;
}

void GCIPair::getDecodedData(void *result, int offset, int size) {
	assert(offset >= 0 && offset + size <= GCI_BLOCK_BODY_SIZE * (GCI_NUM_BLOCKS - 1));
	assert(result != nullptr);

	for (int i = 0; i < size; ++i) {
		int curBlock = (offset + i) / GCI_BLOCK_BODY_SIZE;
		int curBlockOffset = (offset + i) % GCI_BLOCK_BODY_SIZE;
		*((uint8_t *)result + i) = decodedFile->gameBlocks[curBlock].data[curBlockOffset];
		// printf("%2x", *((uint8_t *)result + i));
	}

	// fflush(stdout);

	// assert(*((uint8_t *)result + 0x10) == 'N');
	// assert(*((uint8_t *)result + 0x11) == 'e');
	// assert(*((uint8_t *)result + 0x12) == 'i');
	// assert(*((uint8_t *)result + 0x13) == 'l');
}

void GCIPair::setDecodedData(void *data, int offset, int size) {
	assert(offset >= 0 && offset + size <= GCI_BLOCK_BODY_SIZE * (GCI_NUM_BLOCKS - 1) && data != nullptr);
	for (int i = 0; i < size; ++i) {
		int curBlock = (offset + i) / GCI_BLOCK_BODY_SIZE;
		int curBlockOffset = (offset + i) % GCI_BLOCK_BODY_SIZE;
		decodedFile->gameBlocks[curBlock].data[curBlockOffset] = *((uint8_t *)data + i);
	}
}

bool GCIPair::saveEncodedFile(std::string filename) {
	if (!filename.empty()) {
		this->encodedFileName = filename;
	}
	if (convertGCIFile(encodedFile, decodedFile, initChecksum, GCI_FILE_TYPE_DECODED) == 0) {
		return 0;
	}
	return writeGCIFile(encodedFile, encodedFileName.c_str());
}
