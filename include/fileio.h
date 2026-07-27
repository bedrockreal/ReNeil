#ifndef RENEIL_FILEIO_H
#define RENEIL_FILEIO_H

#include "convert.h"

#include <stdbool.h>

bool readGCIFile(SaveFileData *data, char *inFileName);
bool writeGCIFile(SaveFileData *data, char *outFileName);

#endif
