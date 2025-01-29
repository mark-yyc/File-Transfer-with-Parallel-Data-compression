
#include "parallel_decoder.h"

struct LZWDecodeThreadData
{
    const std::vector<int> *codes;
    std::string decodedOutput;
    int startPos;
};

// Decoding function for each thread
void *lzwDecodeThread(void *arg)
{
    auto *data = (LZWDecodeThreadData *)arg;
    const std::vector<int> &codes = *data->codes;

    std::unordered_map<int, std::string> dictionary;
    for (int i = 0; i < 256; ++i)
    {
        dictionary[i] = std::string(1, i);
    }

    std::string decoded = "";
    int dictSize = 256;
    std::string w;

    for (int i = data->startPos; codes[i] != -1; ++i)
    {
        int code = codes[i];
        std::string entry;

        if (dictionary.count(code))
        {
            entry = dictionary[code];
        }
        else if (code == dictSize)
        {
            entry = w + w[0];
        }

        decoded += entry;

        if (!w.empty())
        {
            dictionary[dictSize++] = w + entry[0];
        }

        w = entry;
    }
    // std::cout << decoded << std::endl;
    data->decodedOutput = decoded;
    return nullptr;
}

// Parallel LZW Decompression
void lzwDecompressParallel(const std::vector<int> &codes, const std::string &outputFile)
{
    if (codes.size() == 0 || codes[0] == -1)
        return;
    std::vector<pthread_t *> threads;
    std::vector<LZWDecodeThreadData *> threadDatas;
    pthread_t *thread = new pthread_t;
    LZWDecodeThreadData *threadData = new LZWDecodeThreadData;
    threadData->codes = &codes;
    threadData->startPos = 0;
    threads.push_back(thread);
    threadDatas.push_back(threadData);
    pthread_create(thread, nullptr, lzwDecodeThread, threadData);
    for (std::size_t i = 0; i < codes.size() - 1; i++)
    {
        if (codes[i] == -1)
        {
            thread = new pthread_t;
            threadData = new LZWDecodeThreadData;
            threadData->codes = &codes;
            threadData->startPos = i + 1;
            threads.push_back(thread);
            threadDatas.push_back(threadData);
            pthread_create(thread, nullptr, lzwDecodeThread, threadData);
        }
    }

    // Wait for threads to finish
    for (std::size_t i = 0; i < threads.size(); ++i)
    {
        pthread_join(*threads[i], nullptr);
    }

    // Write results sequentially to the output file
    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open())
    {
        throw std::runtime_error("Failed to open output file.");
    }

    for (std::size_t i = 0; i < threadDatas.size(); ++i)
    {
        out << threadDatas[i]->decodedOutput;
    }
    // for (int i = 0; i < numThreads; ++i)
    // {
    //     out << threadData[i].decodedOutput;
    // }

    out.close();
    std::cout << "Decoding complete. Results written to " << outputFile << std::endl;
}

void decodeParallel(const std::string &compressedFile, const std::string &outputFile, const std::string &metadataFile)
{
    std::ifstream meta(metadataFile, std::ios::binary);
    HuffmanNode *root = deserializeTree(meta);
    meta.close();

    int paddingLength;
    std::vector<int> lzwCodes = huffmanDecompress(compressedFile, root, paddingLength);
    lzwDecompressParallel(lzwCodes, outputFile);
}
