#ifndef HUFFMAN_NODE_H
#define HUFFMAN_NODE_H

#include <iostream>

struct HuffmanNode
{
    int data;           // Data stored (character or integer)
    int freq;           // Frequency of the data
    HuffmanNode *left;  // Left child
    HuffmanNode *right; // Right child

    HuffmanNode(int d, int f, HuffmanNode *l = nullptr, HuffmanNode *r = nullptr);
};

struct Compare
{
    bool operator()(HuffmanNode *l, HuffmanNode *r);
};

#endif // HUFFMAN_NODE_H