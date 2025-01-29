
#ifndef PARALLEL_ENCODER_H
#define PARALLEL_ENCODER_H

#include <pthread.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>

#include "serial_encoder.h"

std::vector<int> lzwCompressParallel(const std::string &inputFile, int numThreads = 64);
void encodeParallel(const std::string &inputFile,
                    const std::string &compressedFile,
                    const std::string &metadataFile,
                    int numThreads = 64);

#endif
