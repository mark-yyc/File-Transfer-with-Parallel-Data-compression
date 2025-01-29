#include "parallel_encoder.h"
#include "parallel_decoder.h"

size_t getFileSize(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate); // Open file in binary mode and move to the end
    if (file.is_open())
    {
        size_t size = file.tellg(); // Get the position of the cursor (end of the file)
        file.close();
        return size;
    }
    else
    {
        std::cerr << "Unable to open the file.\n";
        return 0;
    }
}

void testCompressParallel(const std::string &filename, int numThreads)
{
    struct timespec start, stop;
    size_t originalSize = getFileSize(filename);
    std::string compressedFile = "./bin/compressed.bin";
    std::string metadataFile = "./bin/metadata.bin";

    std::cout << "Compressing " << filename << " in parallel...\n";
    if (clock_gettime(CLOCK_REALTIME, &start) == -1)
    {
        perror("clock gettime");
    }
    encodeParallel(filename, compressedFile, metadataFile, numThreads);
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1)
    {
        perror("clock gettime");
    }
    double time = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1e9;
    size_t compressedSize = getFileSize(compressedFile);
    std::cout << "execution time: " << time << " sec, compression rate: " << (double)compressedSize / originalSize
              << std::endl;
}

void testCompress(const std::string &filename)
{
    struct timespec start, stop;
    size_t originalSize = getFileSize(filename);
    std::string compressedFile = "./bin/compressed.bin";
    std::string metadataFile = "./bin/metadata.bin";

    std::cout << "Compressing " << filename << "...\n";
    if (clock_gettime(CLOCK_REALTIME, &start) == -1)
    {
        perror("clock gettime");
    }
    encode(filename, compressedFile, metadataFile);
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1)
    {
        perror("clock gettime");
    }
    double time = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1e9;
    size_t compressedSize = getFileSize(compressedFile);
    std::cout << "execution time: " << time << " sec, compression rate: " << (double)compressedSize / originalSize
              << std::endl;
}

int main()
{
    std::string textFile = "./data/english_text_bible.txt";
    std::string artFile = "./data/ascii_art_large.txt";
    std::string randomTextFile = "./data/random_text.txt";
    std::cout << "Test serial version: " << std::endl;
    testCompress(textFile);
    testCompress(artFile);
    // testCompress(randomTextFile);

    std::cout << "Test parallel version: " << std::endl;
    for (int i = 4; i <= 32; i *= 2)
    {
        std::cout << "Number of threads: " << i << std::endl;
        testCompressParallel(textFile, i);
        testCompressParallel(artFile, i);
        // testCompressParallel(randomTextFile, i);
        std::cout << std::endl;
    }
    return 0;
}