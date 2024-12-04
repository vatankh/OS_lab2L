#include "lab2_file.h"
#include <iostream>
#include <cstring>
#include <chrono> // For timing
#include <fcntl.h>
#define BLOCK_SIZE 8192 // 8 KB

// Function to write data using direct system calls (no cache)
void direct_io_write(int repetitions) {
    char buffer[BLOCK_SIZE];
    memset(buffer, 'A', BLOCK_SIZE); // Fill buffer with dummy data

    int fd = open("testfile_direct.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Error opening file");
        exit(1);
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < repetitions; i++) {
        if (write(fd, buffer, BLOCK_SIZE) != BLOCK_SIZE) {
            perror("Error writing to file");
            close(fd);
            exit(1);
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    std::cout << "Direct I/O Write Latency Benchmark: Repetitions = " << repetitions
              << ", Time = " << elapsed_time.count() << " seconds" << std::endl;

    close(fd);
}

// Function to write data using the custom caching system
void cached_io_write(int repetitions) {
    char buffer[BLOCK_SIZE];
    memset(buffer, 'A', BLOCK_SIZE); // Fill buffer with dummy data

    int fd = lab2_open("testfile_cached.dat");
    if (fd < 0) {
        std::cerr << "Error opening file with cache" << std::endl;
        exit(1);
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < repetitions; i++) {
        if (lab2_write(fd, buffer, BLOCK_SIZE) != BLOCK_SIZE) {
            std::cerr << "Error writing to file with cache" << std::endl;
            lab2_close(fd);
            exit(1);
        }
    }

    // Ensure all data is written to disk
    lab2_fsync(fd);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    std::cout << "Cached I/O Write Latency Benchmark: Repetitions = " << repetitions
              << ", Time = " << elapsed_time.count() << " seconds" << std::endl;

    lab2_close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <repetitions>" << std::endl;
        return 1;
    }

    int repetitions = std::atoi(argv[1]);
    if (repetitions <= 0) {
        std::cerr << "Error: Repetitions must be a positive integer" << std::endl;
        return 1;
    }

    std::cout << "Running Direct I/O Benchmark..." << std::endl;
    direct_io_write(repetitions);

    std::cout << "Running Cached I/O Benchmark..." << std::endl;
    cached_io_write(repetitions);

    return 0;
}
