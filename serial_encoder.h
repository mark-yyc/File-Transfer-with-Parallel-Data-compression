#ifndef SERIAL_ENCODER_H
#define SERIAL_ENCODER_H

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <bitset>
#include <string>

#include "huffman_node.h"

std::vector<int> lzwCompress(const std::string &inputFile);
void printLZWCode(const std::vector<int> code);
std::unordered_map<int, int> calculateFrequencies(const std::vector<int> &codes);
HuffmanNode *buildHuffmanTree(const std::unordered_map<int, int> &freqMap);
void generateCodes(HuffmanNode *root, const std::string &code, std::unordered_map<int, std::string> &codes);
void serializeTree(HuffmanNode *root, std::ofstream &out);
void huffmanCompress(const std::vector<int> &codes,
                     const std::string &outputFile,
                     const std::unordered_map<int, std::string> &huffmanCodes,
                     int &paddingLength);
void encode(const std::string &inputFile, const std::string &compressedFile, const std::string &metadataFile);

#endif
