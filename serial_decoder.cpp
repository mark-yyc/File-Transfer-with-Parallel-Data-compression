#include "serial_decoder.h"

void lzwDecompress(const std::vector<int> &codes, const std::string &outputFile)
{
    std::unordered_map<int, std::string> dictionary;
    for (int i = 0; i < 256; ++i)
    {
        dictionary[i] = std::string(1, i);
    }

    std::ofstream out(outputFile, std::ios::binary);
    std::string w = dictionary[codes[0]];
    out << w;
    std::string entry;
    int dictSize = 256;

    for (size_t i = 1; i < codes.size(); ++i)
    {
        int code = codes[i];

        if (dictionary.count(code))
        {
            entry = dictionary[code];
        }
        else if (code == dictSize)
        {
            entry = w + w[0];
        }

        out << entry;
        dictionary[dictSize++] = w + entry[0];
        w = entry;
    }

    out.close();
}

HuffmanNode *deserializeTree(std::ifstream &in)
{
    char nodeType;
    in.get(nodeType);

    if (nodeType == '1')
    { // Leaf node
        int data;
        in.read(reinterpret_cast<char *>(&data), sizeof(int));
        return new HuffmanNode(data, 0);
    }

    // Internal node
    HuffmanNode *left = deserializeTree(in);
    HuffmanNode *right = deserializeTree(in);
    return new HuffmanNode(-1, 0, left, right);
}

std::vector<int> huffmanDecompress(const std::string &inputFile, HuffmanNode *root, int &paddingLength)
{
    std::ifstream in(inputFile, std::ios::binary);
    std::vector<int> codes;
    std::string bitString = "";

    // Read the padding length
    char padLength;
    in.get(padLength);
    paddingLength = static_cast<int>(padLength);

    char byte;
    while (in.get(byte))
    {
        bitString += std::bitset<8>(static_cast<unsigned char>(byte)).to_string();
    }

    // Remove padding bits
    bitString = bitString.substr(0, bitString.size() - paddingLength);

    HuffmanNode *current = root;
    for (char bit : bitString)
    {
        current = (bit == '0') ? current->left : current->right;

        if (!current->left && !current->right)
        {
            codes.push_back(current->data);
            current = root;
        }
    }

    in.close();
    return codes;
}

void decode(const std::string &compressedFile, const std::string &outputFile, const std::string &metadataFile)
{
    std::ifstream meta(metadataFile, std::ios::binary);
    HuffmanNode *root = deserializeTree(meta);
    meta.close();

    int paddingLength;
    std::vector<int> lzwCodes = huffmanDecompress(compressedFile, root, paddingLength);
    lzwDecompress(lzwCodes, outputFile);
}