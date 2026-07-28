#ifndef RENEIL_GCI_PAIR_HPP
#define RENEIL_GCI_PAIR_HPP

#include <cstdint>
#include <string>
extern "C" {
#include "gci.h"
}

struct GCIPair {
	public:
		GCIPair();
		GCIPair(std::string encodedFileName, uint32_t initChecksum = 0x12345678);
		~GCIPair();
		void init(std::string encodedFileName, uint32_t initChecksum);
		bool isInitSuccess();

		void getDecodedData(void *result, int offset, int size);
		void setDecodedData(uint8_t *data, int offset, int size);
		bool saveEncodedFile(std::string filename);

	private:
		GCIFile *encodedFile;
		GCIFile *decodedFile;
		std::string encodedFileName;
		uint32_t initChecksum;
		bool initSuccess;
};

#endif
