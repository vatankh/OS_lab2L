#include <fcntl.h>     // For open, O_* flags
#include <unistd.h>    // For close, read, write, lseek
#include <cstring>     // For strerror
#include <iostream>
#include <unordered_map>
#include <vector>
#include "lab2_lru.cpp" // Include the LRU cache implementation

class Lab2Cache {
private:
    // Cache capacity in number of blocks
    size_t cache_capacity;

    // Block size (in bytes)
    size_t block_size;

    // LRU cache for blocks
    LRUCache lru_cache;

    // Mapping of file descriptors to open file paths
    std::unordered_map<int, std::string> open_files;

public:
    // Constructor
    explicit Lab2Cache(size_t capacity, size_t block_size)
        : cache_capacity(capacity), block_size(block_size), lru_cache(capacity) {}

    // Open a file and register it in the cache
    int cache_open(const std::string& path) {
        int fd = open(path.c_str(), O_RDWR | O_SYNC);
        if (fd == -1) {
            perror("Error opening file");
            return -1;
        }
        open_files[fd] = path;
        return fd;
    }

    // Close a file and remove it from the cache
    int cache_close(int fd) {
        if (open_files.find(fd) == open_files.end()) {
            std::cerr << "Error: Invalid file descriptor" << std::endl;
            return -1;
        }

        // Sync all cached blocks for this file
        cache_fsync(fd);

        // Remove the file descriptor from the mapping
        open_files.erase(fd);

        return close(fd);
    }

    // Read data from a file with caching
    ssize_t cache_read(int fd, void* buf, size_t count) {
        if (open_files.find(fd) == open_files.end()) {
            std::cerr << "Error: Invalid file descriptor" << std::endl;
            return -1;
        }

        char* buffer = static_cast<char*>(buf);
        size_t bytes_read = 0;

        while (bytes_read < count) {
            off_t offset = lseek(fd, 0, SEEK_CUR);
            int block_id = offset / block_size;
            size_t block_offset = offset % block_size;

            // Check if the block is in the cache
            auto cached_block = lru_cache.get_page(block_id);

            if (cached_block) {
                // Block is in cache; copy data
                size_t to_read = std::min(block_size - block_offset, count - bytes_read);
                std::memcpy(buffer + bytes_read, cached_block->data() + block_offset, to_read);
                bytes_read += to_read;

                // Adjust the file pointer
                lseek(fd, to_read, SEEK_CUR);
            } else {
                // Block is not in cache; read from disk
                std::vector<char> block(block_size, 0);
                ssize_t result = pread(fd, block.data(), block_size, block_id * block_size);

                if (result <= 0) {
                    return bytes_read > 0 ? bytes_read : result;
                }

                // Cache the block
                lru_cache.add_page(block_id, block);

                // Copy data to the output buffer
                size_t to_read = std::min(block_size - block_offset, count - bytes_read);
                std::memcpy(buffer + bytes_read, block.data() + block_offset, to_read);
                bytes_read += to_read;

                // Adjust the file pointer
                lseek(fd, to_read, SEEK_CUR);
            }
        }

        return bytes_read;
    }

    // Write data to a file with caching
    ssize_t cache_write(int fd, const void* buf, size_t count) {
        if (open_files.find(fd) == open_files.end()) {
            std::cerr << "Error: Invalid file descriptor" << std::endl;
            return -1;
        }

        const char* buffer = static_cast<const char*>(buf);
        size_t bytes_written = 0;

        while (bytes_written < count) {
            off_t offset = lseek(fd, 0, SEEK_CUR);
            int block_id = offset / block_size;
            size_t block_offset = offset % block_size;

            // Check if the block is in the cache
            auto cached_block = lru_cache.get_page(block_id);

            if (!cached_block) {
                // If not in cache, load the block from disk
                cached_block = std::vector<char>(block_size, 0);
                ssize_t result = pread(fd, cached_block->data(), block_size, block_id * block_size);
            if (result == -1) {
                        perror("Error reading block from file");
            return -1; // Return error immediately
        }           
         }

            // Modify the block in the cache
            size_t to_write = std::min(block_size - block_offset, count - bytes_written);
            std::memcpy(cached_block->data() + block_offset, buffer + bytes_written, to_write);
            lru_cache.add_page(block_id, *cached_block); // Update the cache
            bytes_written += to_write;

            // Adjust the file pointer
            lseek(fd, to_write, SEEK_CUR);
        }

        return bytes_written;
    }

    // Seek to a specific position in the file
    off_t cache_lseek(int fd, off_t offset, int whence) {
        if (open_files.find(fd) == open_files.end()) {
            std::cerr << "Error: Invalid file descriptor" << std::endl;
            return -1;
        }

        return lseek(fd, offset, whence);
    }

    // Sync cached data for a file to the disk
    int cache_fsync(int fd) {
        if (open_files.find(fd) == open_files.end()) {
            std::cerr << "Error: Invalid file descriptor" << std::endl;
            return -1;
        }

        // Iterate over cached pages and write them to disk
        for (const auto& [block_id, data] : lru_cache.get_cache_contents()) {
            off_t offset = block_id * block_size;
            ssize_t result = pwrite(fd, data.data(), data.size(), offset);

            if (result == -1) {
                perror("Error syncing block");
                return -1;
            }
        }

        return fsync(fd);
    }
};
