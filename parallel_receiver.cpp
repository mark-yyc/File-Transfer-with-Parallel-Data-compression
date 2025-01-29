#include <iostream>
#include <fstream>
#include <map>
#include <pthread.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
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
    std::map<int, Chunk> *chunkBuffer;
    pthread_mutex_t *bufferMutex;
    pthread_cond_t *condition;
    int *expectedSequence;
    bool *transferComplete;
    bool *threadsActive;
};

void *receiveChunk(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    Chunk chunk;

    while (true)
    {
        if (recv(data->socket, &chunk, sizeof(chunk), 0) <= 0)
        {
            break; // Socket error or closed connection
        }

        pthread_mutex_lock(data->bufferMutex);

        if (chunk.sequence == END_MARKER)
        {
            *data->transferComplete = true;
            pthread_cond_broadcast(data->condition); // Notify all threads of completion
            pthread_mutex_unlock(data->bufferMutex);
            break;
        }

        // Add chunk to buffer
        (*data->chunkBuffer)[chunk.sequence] = chunk;
        pthread_cond_signal(data->condition);
        pthread_mutex_unlock(data->bufferMutex);
    }

    pthread_mutex_lock(data->bufferMutex);
    *data->threadsActive = false;            // Signal this thread is done
    pthread_cond_broadcast(data->condition); // Notify main thread
    pthread_mutex_unlock(data->bufferMutex);

    pthread_exit(nullptr);
}

void writeChunksToFile(std::ofstream &outFile,
                       std::map<int, Chunk> &chunkBuffer,
                       pthread_mutex_t *bufferMutex,
                       pthread_cond_t *condition,
                       int *expectedSequence,
                       bool *transferComplete,
                       bool *threadsActive)
{
    while (true)
    {
        pthread_mutex_lock(bufferMutex);

        // Wait until the next expected chunk is available or transfer is complete.
        while (!*transferComplete && chunkBuffer.find(*expectedSequence) == chunkBuffer.end())
        {
            pthread_cond_wait(condition, bufferMutex);
        }

        if (*transferComplete && chunkBuffer.find(*expectedSequence) == chunkBuffer.end())
        {
            if (!*threadsActive)
            { // No active threads, safe to exit
                pthread_mutex_unlock(bufferMutex);
                break;
            }
        }

        // Write the chunk to the file in sequence if available.
        if (chunkBuffer.find(*expectedSequence) != chunkBuffer.end())
        {
            Chunk chunk = chunkBuffer[*expectedSequence];
            outFile.write(chunk.data, chunk.size);
            chunkBuffer.erase(*expectedSequence);
            (*expectedSequence)++;
        }

        pthread_mutex_unlock(bufferMutex);
    }
}

void receiveFileParallel(int socket, const std::string &fileName)
{
    std::ofstream outFile(fileName, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Error creating file!" << std::endl;
        return;
    }

    pthread_t threads[THREAD_COUNT];
    pthread_mutex_t bufferMutex;
    pthread_cond_t condition;
    pthread_mutex_init(&bufferMutex, nullptr);
    pthread_cond_init(&condition, nullptr);

    std::map<int, Chunk> chunkBuffer;
    int expectedSequence = 0;
    bool transferComplete = false;
    bool threadsActive = true;

    ThreadData data = {
        socket, &chunkBuffer, &bufferMutex, &condition, &expectedSequence, &transferComplete, &threadsActive};

    // Start threads to receive chunks.
    for (int i = 0; i < THREAD_COUNT; ++i)
    {
        pthread_create(&threads[i], nullptr, receiveChunk, &data);
    }

    // Write chunks to the file in sequence.
    writeChunksToFile(
        outFile, chunkBuffer, &bufferMutex, &condition, &expectedSequence, &transferComplete, &threadsActive);

    // Cleanup
    for (int i = 0; i < THREAD_COUNT; ++i)
    {
        pthread_join(threads[i], nullptr);
    }
    pthread_mutex_destroy(&bufferMutex);
    pthread_cond_destroy(&condition);
    outFile.close();
}

int main(int argc, char *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

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

    std::cout << "Waiting for connection..." << std::endl;

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Connection established!" << std::endl;
    receiveFileParallel(new_socket, argv[2]);

    close(new_socket);
    close(server_fd);

    return 0;
}
