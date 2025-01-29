#ifndef SERIAL_DECODER_H
#define SERIAL_DECODER_H

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <bitset>
#include <string>

#include "huffman_node.h"

void lzwDecompress(const std::vector<int> &codes, const std::string &outputFile);
HuffmanNode *deserializeTree(std::ifstream &in);
std::vector<int> huffmanDecompress(const std::string &inputFile, HuffmanNode *root, int &paddingLength);
void decode(const std::string &compressedFile, const std::string &outputFile, const std::string &metadataFile);

#endif
