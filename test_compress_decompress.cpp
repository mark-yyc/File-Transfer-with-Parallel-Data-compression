#include "parallel_encoder.h"
#include "parallel_decoder.h"

int main()
{
    std::string inputFile = "./data/english_text_bible.txt";
    std::string compressedFile = "./bin/compressed.bin";
    std::string metadataFile = "./bin/metadata.bin";
    std::string outputFile = "./bin/output.txt";

    std::cout << "Compressing " << inputFile << " ...\n";
    encodeParallel(inputFile, compressedFile, metadataFile);
    std::cout << "Decompressing " << inputFile << " ...\n";
    decodeParallel(compressedFile, outputFile, metadataFile);
    std::cout << "Compression and decompression completed successfully.\n";
    return 0;
}