#include "huffman_node.h"

HuffmanNode::HuffmanNode(int d, int f, HuffmanNode *l, HuffmanNode *r)
: data(d)
, freq(f)
, left(l)
, right(r)
{
}

bool Compare::operator()(HuffmanNode *l, HuffmanNode *r)
{
    return l->freq > r->freq; // Min-heap
}