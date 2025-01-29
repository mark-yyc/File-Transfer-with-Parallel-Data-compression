
#ifndef PARALLEL_DECODER_H
#define PARALLEL_DECODER_H

#include <pthread.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>

#include "serial_decoder.h"

void lzwDecompressParallel(const std::vector<int> &codes, const std::string &outputFile);
void decodeParallel(const std::string &compressedFile, const std::string &outputFile, const std::string &metadataFile);

#endif
