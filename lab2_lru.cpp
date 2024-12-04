#include <unordered_map>
#include <list>
#include <vector>
#include <optional>
#include <iostream>

class LRUCache {
private:
    // The maximum capacity of the cache
    size_t capacity;

    // Doubly linked list to maintain the order of pages (most recently used at the front)
    std::list<int> lru_list;

    // Hashmap to map page_id to its position in the list and its data
    std::unordered_map<int, std::pair<std::list<int>::iterator, std::vector<char>>> cache;

public:
    // Constructor
    explicit LRUCache(size_t cache_capacity) : capacity(cache_capacity) {}

    // Add a page to the cache
    void add_page(int page_id, const std::vector<char>& data) {
        // Check if the page already exists
        if (cache.find(page_id) != cache.end()) {
            // Update the page's data and move it to the front
            cache[page_id].second = data;
            move_to_front(page_id);
            return;
        }

        // If the cache is full, evict the least recently used page
        if (cache.size() >= capacity) {
            evict_lru();
        }

        // Add the new page to the front of the LRU list and insert it into the cache
        lru_list.push_front(page_id);
        cache[page_id] = {lru_list.begin(), data};
    }

    // Retrieve a page from the cache
    std::optional<std::vector<char>> get_page(int page_id) {
        if (cache.find(page_id) == cache.end()) {
            // Page not found
            return std::nullopt;
        }

        // Move the page to the front of the LRU list
        move_to_front(page_id);
        return cache[page_id].second;
    }

    // Remove a page from the cache
    void remove_page(int page_id) {
        auto it = cache.find(page_id);
        if (it != cache.end()) {
            // Erase the page from the LRU list and the cache
            lru_list.erase(it->second.first);
            cache.erase(it);
        }
    }

    // Check if a page exists in the cache
    bool contains(int page_id) const {
        return cache.find(page_id) != cache.end();
    }

    // Print the current LRU order (for debugging purposes)
    void print_cache_order() const {
        std::cout << "LRU Order: ";
        for (int page_id : lru_list) {
            std::cout << page_id << " ";
        }
        std::cout << std::endl;
    }
        // ** New Method: get_cache_contents **
    std::unordered_map<int, std::vector<char>> get_cache_contents() const {
        std::unordered_map<int, std::vector<char>> contents;
        for (const auto& [page_id, entry] : cache) {
            contents[page_id] = entry.second;
        }
        return contents;
    }

private:
    // Move a page to the front of the LRU list
    void move_to_front(int page_id) {
        auto it = cache[page_id].first;
        lru_list.erase(it);
        lru_list.push_front(page_id);
        cache[page_id].first = lru_list.begin();
    }

    // Evict the least recently used page
    void evict_lru() {
        int lru_page_id = lru_list.back();
        lru_list.pop_back();
        cache.erase(lru_page_id);
    }
};
