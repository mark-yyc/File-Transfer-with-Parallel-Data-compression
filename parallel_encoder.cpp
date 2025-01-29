#include "parallel_encoder.h"

struct LZWThreadData
{
    std::string inputFile;
    std::vector<int> *codes;
    int startPos;
    int endPos;
};

// LZW Compression function
void *lzwCompressThread(void *arg)
{
    LZWThreadData *data = (LZWThreadData *)arg;
    std::ifstream in(data->inputFile, std::ios::binary);
    std::unordered_map<std::string, int> dictionary;
    std::string w;
    char c;
    int dictSize = 256;

    // Initialize dictionary with single characters
    for (int i = 0; i < 256; ++i)
    {
        dictionary[std::string(1, i)] = i;
    }

    in.seekg(data->startPos);
    int position = data->startPos;

    while (in.get(c) && position < data->endPos)
    {
        std::string wc = w + c;
        if (dictionary.count(wc))
        {
            w = wc;
        }
        else
        {
            data->codes->push_back(dictionary[w]);
            dictionary[wc] = dictSize++;
            w = std::string(1, c);
        }
        position++;
    }

    if (!w.empty())
    {
        data->codes->push_back(dictionary[w]);
    }
    data->codes->push_back(-1);
    in.close();
    return nullptr;
}

// Main function for parallel LZW compression
std::vector<int> lzwCompressParallel(const std::string &inputFile, int numThreads)
{
    std::ifstream in(inputFile, std::ios::binary | std::ios::ate);
    std::streamsize fileSize = in.tellg();
    int chunkSize = fileSize / numThreads;
    pthread_t threads[numThreads];
    std::vector<int> codes[numThreads]; // Array to hold results from each thread

    // Launch threads
    for (int i = 0; i < numThreads; ++i)
    {
        LZWThreadData *data = new LZWThreadData;
        data->inputFile = inputFile;
        data->codes = &codes[i];
        data->startPos = i * chunkSize;
        data->endPos = (i == numThreads - 1) ? fileSize : (i + 1) * chunkSize;

        pthread_create(&threads[i], nullptr, lzwCompressThread, data);
    }

    // Wait for all threads to complete
    for (int i = 0; i < numThreads; ++i)
    {
        pthread_join(threads[i], nullptr);
    }

    // Combine results from each thread
    std::vector<int> allCodes;
    for (int i = 0; i < numThreads; ++i)
    {
        allCodes.insert(allCodes.end(), codes[i].begin(), codes[i].end());
    }

    return allCodes;
}

// Structure for passing parameters to the frequency calculation thread
struct FrequencyThreadData
{
    std::vector<int> codes;
    std::unordered_map<int, int> *freqMap;
    int startPos;
    int endPos;
};

// Frequency calculation function
void *calculateFrequenciesThread(void *arg)
{
    FrequencyThreadData *data = (FrequencyThreadData *)arg;
    for (int i = data->startPos; i < data->endPos; ++i)
    {
        data->freqMap->operator[](data->codes[i])++;
    }
    return nullptr;
}

// Main function for parallel frequency calculation
std::unordered_map<int, int> calculateFrequenciesParallel(const std::vector<int> &codes, int numThreads)
{
    int chunkSize = codes.size() / numThreads;
    pthread_t threads[numThreads];
    std::unordered_map<int, int> freqMap[numThreads]; // Array to hold results from each thread

    // Launch threads for frequency calculation
    for (int i = 0; i < numThreads; ++i)
    {
        FrequencyThreadData *data = new FrequencyThreadData;
        data->codes = codes;
        data->freqMap = &freqMap[i];
        data->startPos = i * chunkSize;
        data->endPos = (i == numThreads - 1) ? codes.size() : (i + 1) * chunkSize;

        pthread_create(&threads[i], nullptr, calculateFrequenciesThread, data);
    }

    // Wait for all threads to complete
    for (int i = 0; i < numThreads; ++i)
    {
        pthread_join(threads[i], nullptr);
    }

    // Merge results from each thread
    std::unordered_map<int, int> finalFreqMap;
    for (int i = 0; i < numThreads; ++i)
    {
        for (const auto &entry : freqMap[i])
        {
            finalFreqMap[entry.first] += entry.second;
        }
    }

    return finalFreqMap;
}

void encodeParallel(const std::string &inputFile,
                    const std::string &compressedFile,
                    const std::string &metadataFile,
                    int numThreads)
{
    std::vector<int> lzwCodes = lzwCompressParallel(inputFile, numThreads);
    std::unordered_map<int, int> freqMap = calculateFrequenciesParallel(lzwCodes, numThreads);
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