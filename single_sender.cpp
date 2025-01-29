#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "parallel_encoder.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void sendFile(int socket, const std::string &fileName)
{
    std::ifstream inFile(fileName, std::ios::binary);
    if (!inFile)
    {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    while (inFile.read(buffer, sizeof(buffer)))
    {
        ssize_t bytesSent = send(socket, buffer, inFile.gcount(), 0);
        if (bytesSent == -1)
        {
            std::cerr << "Error sending data!" << std::endl;
            return;
        }
    }

    if (inFile.gcount() > 0)
    {
        ssize_t bytesSent = send(socket, buffer, inFile.gcount(), 0);
        if (bytesSent == -1)
        {
            std::cerr << "Error sending remaining data!" << std::endl;
            return;
        }
    }

    std::cout << "File " << fileName << " sent successfully." << std::endl;
}

int main()
{
    struct timespec start, stop;
    std::string inputFile = "english_text_bible.txt";
    std::string compressedFile = "compressed.bin";
    std::string metadataFile = "metadata.bin";

    int sock1 = 0, sock2 = 0;
    struct sockaddr_in serv_addr;

    // Setup first socket (for first file)
    if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error for first file" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address" << std::endl;
        return -1;
    }

    if (connect(sock1, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed for first file" << std::endl;
        return -1;
    }

    std::cout << "Connected to server for first file!" << std::endl;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1)
    {
        perror("clock gettime");
    }
    encodeParallel(inputFile, compressedFile, metadataFile);

    // Send first file
    sendFile(sock1, compressedFile);

    // Close first socket after sending the first file
    close(sock1);

    // Setup second socket (for second file)
    if ((sock2 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error for second file" << std::endl;
        return -1;
    }

    if (connect(sock2, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed for second file" << std::endl;
        return -1;
    }

    std::cout << "Connected to server for second file!" << std::endl;

    // Send second file
    sendFile(sock2, metadataFile);
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1)
    {
        perror("clock gettime");
    }
    // Close second socket after sending the second file
    close(sock2);
    double time = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1e9;

    std::cout << "execution time: " << time << std::endl;

    return 0;
}