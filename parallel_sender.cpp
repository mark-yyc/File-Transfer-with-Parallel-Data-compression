#include <iostream>
#include <fstream>
#include <pthread.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define THREAD_COUNT 4
#define END_MARKER -1 // Special marker to indicate end of transmission.

struct Chunk
{
    int sequence;
    size_t size;
    char data[BUFFER_SIZE];
};

struct ThreadData
{
    int socket;
    std::ifstream *inFile;
    pthread_mutex_t *fileMutex;
    int *sequenceCounter;
};

void *sendChunk(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    while (true)
    {
        pthread_mutex_lock(data->fileMutex);

        if (data->inFile->eof())
        {
            pthread_mutex_unlock(data->fileMutex);
            break;
        }

        Chunk chunk;
        chunk.sequence = (*data->sequenceCounter)++;
        data->inFile->read(chunk.data, BUFFER_SIZE);
        chunk.size = data->inFile->gcount();

        pthread_mutex_unlock(data->fileMutex);

        send(data->socket, &chunk, sizeof(chunk), 0);
    }

    pthread_exit(nullptr);
}

void sendFileParallel(int socket, const std::string &fileName)
{
    std::ifstream inFile(fileName, std::ios::binary);
    if (!inFile)
    {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    pthread_t threads[THREAD_COUNT];
    pthread_mutex_t fileMutex;
    pthread_mutex_init(&fileMutex, nullptr);

    int sequenceCounter = 0;

    ThreadData data = {socket, &inFile, &fileMutex, &sequenceCounter};

    // Start threads to send chunks.
    for (int i = 0; i < THREAD_COUNT; ++i)
    {
        pthread_create(&threads[i], nullptr, sendChunk, &data);
    }

    // Wait for all threads to finish.
    for (int i = 0; i < THREAD_COUNT; ++i)
    {
        pthread_join(threads[i], nullptr);
    }

    // Send end-of-transmission marker.
    Chunk endMarker = {END_MARKER, 0, {}};
    send(socket, &endMarker, sizeof(endMarker), 0);

    pthread_mutex_destroy(&fileMutex);
    inFile.close();
}

int main(int argc, char *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address/Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    std::cout << "Connected to server!" << std::endl;
    sendFileParallel(sock, argv[2]);

    close(sock);

    return 0;
}
