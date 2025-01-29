#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "parallel_decoder.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void receiveFile(int socket, const std::string &fileName)
{
    std::ofstream outFile(fileName, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Error creating file: " << fileName << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesReceived;
    while ((bytesReceived = recv(socket, buffer, sizeof(buffer), 0)) > 0)
    {
        outFile.write(buffer, bytesReceived);
    }

    if (bytesReceived == -1)
    {
        std::cerr << "Error receiving data!" << std::endl;
    }

    std::cout << "File " << fileName << " received successfully." << std::endl;
}

int main()
{
    std::string compressedFile = "compressed.bin";
    std::string metadataFile = "metadata.bin";
    std::string outputFile = "output.txt";
    int server_fd, new_socket1, new_socket2;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Setup server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Waiting for connections..." << std::endl;

    // Accept first connection for first file
    if ((new_socket1 = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("Accept failed for first socket");
        exit(EXIT_FAILURE);
    }

    std::cout << "Connection established for first file!" << std::endl;

    // Receive first file
    receiveFile(new_socket1, compressedFile);

    close(new_socket1);

    // Accept second connection for second file
    if ((new_socket2 = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("Accept failed for second socket");
        exit(EXIT_FAILURE);
    }

    std::cout << "Connection established for second file!" << std::endl;

    // Receive second file
    receiveFile(new_socket2, metadataFile);

    decodeParallel(compressedFile, outputFile, metadataFile);

    close(new_socket2);
    close(server_fd);

    return 0;
}