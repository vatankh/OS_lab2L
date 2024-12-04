#include "lab2_cache.cpp" // Include the cache implementation
#include "lab2_file.h"

// Initialize a global cache instance
// Cache capacity: 100 blocks, Block size: 4096 bytes
Lab2Cache global_cache(100, 4096);

// Open a file and register it in the cache
int lab2_open(const char *path) {
    if (!path) {
        std::cerr << "Error: File path is null" << std::endl;
        return -1;
    }

    return global_cache.cache_open(std::string(path));
}

// Close a file and remove it from the cache
int lab2_close(int fd) {
    return global_cache.cache_close(fd);
}

// Read data from a file with caching
ssize_t lab2_read(int fd, void *buf, size_t count) {
    if (!buf) {
        std::cerr << "Error: Buffer is null" << std::endl;
        return -1;
    }

    return global_cache.cache_read(fd, buf, count);
}

// Write data to a file with caching
ssize_t lab2_write(int fd, const void *buf, size_t count) {
    if (!buf) {
        std::cerr << "Error: Buffer is null" << std::endl;
        return -1;
    }

    return global_cache.cache_write(fd, buf, count);
}

// Seek to a specific position in the file
off_t lab2_lseek(int fd, off_t offset, int whence) {
    return global_cache.cache_lseek(fd, offset, whence);
}

// Sync cached data for a file to the disk
int lab2_fsync(int fd) {
    return global_cache.cache_fsync(fd);
}
