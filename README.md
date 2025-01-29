## Optimized File Transfer with Parallel Data compression

Group Members:

- Yicheng Yang

### Introduction

The Project introduces a custom file transfer protocol that integrates data compression with parallel transfer, optimizing both speed and efficiency. The proposed approach leverages the strengths of modern multi-core systems and efficient compression algorithms to minimize transfer time while maintaining data integrity.

In this project, several work will be done:

- Investigate Data Compression for Specific File Patterns
  Analyze a targeted file type and explore data compression algorithms to identify the one that best suits its characteristics in terms of compression ratio, speed, and compatibility.

- Develop a Serial Compression Implementation in C++
  Implement the selected data compression algorithm in C++ to create a functional baseline version.

- Parallelize the Compression Algorithm
  Optimize the serial implementation by incorporating parallelization techniques, leveraging multi-threading or multi-processing to enhance performance.

- Design and Implement a TCP-Based File Transfer Protocol
  Develop a custom file transfer protocol using TCP sockets in C++. Integrate the parallelized compression to streamline the file transfer process and improve throughput.

- Evaluate Performance Across Datasets
  Test the implementations on multiple datasets to evaluate performance metrics such as speed, efficiency, and scalability. Compare results against baseline implementations to validate improvements.

More details can be seen in [Report.md](./report/Report.md)

### File structure

1. serial_encoder, serial_decoder
   Serial version of data compression/decompression
2. parallel_encoder, parallel_decoder
   Parallel version of data compression/decompression
3. single_sender, single_receiver
   Serial version of TCP socket based file transfer
4. parallel_sender, parallel_receiver
   Parallel version of TCP socket based file transfer
5. /report
   All the original resources for my project, including a html and markdown version of report.
6. /data
   Dataset for my project

### Instructions for Executing:

Just run run.sh in the folder, it will do the following steps:

- Compile
- Create a new folder "bin" that is used for storing metadata and compressed file
- Generate a random text file (one of dataset)
- Test on compression and decompression
- Check consistency between output file and original file Using md5, this step might require sudo
- Test on data compression in different number of thread
- Clean all the generating file and directory
