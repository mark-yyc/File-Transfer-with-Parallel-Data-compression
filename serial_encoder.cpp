

#include "serial_encoder.h"

std::vector<int> lzwCompress(const std::string &inputFile)
{
    std::ifstream in(inputFile, std::ios::binary);
    std::unordered_map<std::string, int> dictionary;
    std::vector<int> codes;

    // Initialize dictionary with single characters
    for (int i = 0; i < 256; ++i)
    {
        dictionary[std::string(1, i)] = i;
    }

    std::string w;
    char c;
    int dictSize = 256;

    while (in.get(c))
    {
        std::string wc = w + c;
        if (dictionary.count(wc))
        {
            w = wc;
        }
        else
        {
            codes.push_back(dictionary[w]);
            dictionary[wc] = dictSize++;
            w = std::string(1, c);
        }
    }

    if (!w.empty())
    {
        codes.push_back(dictionary[w]);
    }

    in.close();
    return codes;
}

void printLZWCode(const std::vector<int> code)
{
    std::cout << "LZW code: ";
    for (int i : code)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}
// --- Frequency Calculation ---
std::unordered_map<int, int> calculateFrequencies(const std::vector<int> &codes)
{
    std::unordered_map<int, int> freqMap;
    for (int code : codes)
    {
        freqMap[code]++;
    }
    return freqMap;
}

// --- Build Huffman Tree ---
HuffmanNode *buildHuffmanTree(const std::unordered_map<int, int> &freqMap)
{
    std::priority_queue<HuffmanNode *, std::vector<HuffmanNode *>, Compare> minHeap;

    for (const auto &entry : freqMap)
    {
        minHeap.push(new HuffmanNode(entry.first, entry.second));
    }

    while (minHeap.size() > 1)
    {
        HuffmanNode *left = minHeap.top();
        minHeap.pop();
        HuffmanNode *right = minHeap.top();
        minHeap.pop();
        HuffmanNode *combined = new HuffmanNode(-1, left->freq + right->freq, left, right);
        minHeap.push(combined);
    }

    return minHeap.top();
}

// --- Generate Huffman Codes ---
void generateCodes(HuffmanNode *root, const std::string &code, std::unordered_map<int, std::string> &codes)
{
    if (!root)
        return;
    if (!root->left && !root->right)
    {
        codes[root->data] = code;
    }

    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}

// --- Serialize Huffman Tree ---
void serializeTree(HuffmanNode *root, std::ofstream &out)
{
    if (!root)
        return;

    if (!root->left && !root->right)
    {
        out.put('1'); // Leaf node
        out.write(reinterpret_cast<const char *>(&root->data), sizeof(int));
    }
    else
    {
        out.put('0'); // Internal node
    }

    serializeTree(root->left, out);
    serializeTree(root->right, out);
}

void huffmanCompress(const std::vector<int> &codes,
                     const std::string &outputFile,
                     const std::unordered_map<int, std::string> &huffmanCodes,
                     int &paddingLength)
{
    std::ofstream out(outputFile, std::ios::binary);
    std::string bitString = "";

    for (int code : codes)
    {
        bitString += huffmanCodes.at(code);
    }

    // Pad bitstring to make it a multiple of 8
    paddingLength = 8 - (bitString.size() % 8);
    while (bitString.size() % 8 != 0)
    {
        bitString += "0";
    }

    // Write padding length (1 byte)
    out.put(static_cast<char>(paddingLength));

    // Write bitstring as bytes
    for (size_t i = 0; i < bitString.size(); i += 8)
    {
        std::bitset<8> byte(bitString.substr(i, 8));
        out.put(static_cast<char>(byte.to_ulong()));
    }

    out.close();
}

void encode(const std::string &inputFile, const std::string &compressedFile, const std::string &metadataFile)
{
    std::vector<int> lzwCodes = lzwCompress(inputFile);
    std::unordered_map<int, int> freqMap = calculateFrequencies(lzwCodes);
    HuffmanNode *root = buildHuffmanTree(freqMap);

    std::unordered_map<int, std::string> huffmanCodes;
    generateCodes(root, "", huffmanCodes);

    std::ofstream meta(metadataFile, std::ios::binary);
    serializeTree(root, meta);
    meta.close();

    int paddingLength;
    huffmanCompress(lzwCodes, compressedFile, huffmanCodes, paddingLength);

    // Save padding length in metadata file
    std::ofstream metaOut(metadataFile, std::ios::binary | std::ios::app);
    metaOut.put(static_cast<char>(paddingLength));
    metaOut.close();
}
