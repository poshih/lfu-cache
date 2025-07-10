/*
 * Simple Usage Example for High-Performance LFU Cache
 * 
 * MIT License - Copyright (c) 2024 Po Shih Tsang
 * 
 * Compile with: g++ -std=c++17 -O3 -march=native simple_example.cpp -o example
 */

#include "lfu_cache.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "High-Performance LFU Cache Example\n";
    std::cout << "==================================\n\n";
    
    // Create cache with <Key, Value, MaxSize> template parameters
    // This cache can hold up to 500 elements, with max 1000 pre-allocated
    LFUCache<int, std::string, 1000> cache;
    
    std::cout << "1. Basic Operations:\n";
    
    // Add some data
    cache.put(1, "First");
    cache.put(2, "Second");
    cache.put(3, "Third");
    
    std::cout << "   Added 3 items, cache size: " << cache.size() << "\n";
    
    // Access data (increases frequency)
    std::cout << "   Key 1: " << cache.get(1) << " (noexcept access)\n";
    std::cout << "   Key 2: " << cache.getOrDefault(2, "Not found") << "\n";
    
    // Access key 1 again to increase its frequency
    cache.get(1);
    
    std::cout << "\n2. Error Handling:\n";
    
    // Safe access for missing keys
    std::cout << "   Missing key (safe): '" << cache.get(999) << "' (empty string)\n";
    std::cout << "   Missing key (fallback): " << cache.getOrDefault(999, "Default") << "\n";
    
    // Exception-based access when needed
    try {
        auto value = cache.getOrThrow(999);
        std::cout << "   This won't print\n";
    } catch (const std::runtime_error& e) {
        std::cout << "   Exception caught: " << e.what() << "\n";
    }
    
    std::cout << "\n3. Performance Test:\n";
    
    // Fill cache to demonstrate LFU eviction
    for (int i = 4; i <= 503; ++i) {  // Will exceed capacity of 500
        cache.put(i, "Item" + std::to_string(i));
    }
    
    std::cout << "   After adding 500+ items, cache size: " << cache.size() << "\n";
    std::cout << "   Cache capacity: " << cache.capacity() << "\n";
    
    // Check if frequently accessed items survived
    std::cout << "   Key 1 (accessed 3 times): ";
    if (cache.contains(1)) {
        std::cout << "Still in cache: " << cache.get(1) << "\n";
    } else {
        std::cout << "Evicted (LFU)\n";
    }
    
    std::cout << "   Key 3 (accessed 1 time): ";
    if (cache.contains(3)) {
        std::cout << "Still in cache: " << cache.get(3) << "\n";
    } else {
        std::cout << "Evicted (LFU)\n";
    }
    
    std::cout << "\n4. Performance Demonstration:\n";
    
    // Hot loop - all noexcept for maximum performance
    const int HOT_KEYS[] = {1, 100, 200, 300, 400};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < 100000; ++iter) {
        for (int key : HOT_KEYS) {
            if (cache.contains(key)) {           // noexcept
                volatile auto value = cache.get(key);  // noexcept, maximum performance
                (void)value;  // Prevent optimization
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "   Performed 500,000 cache operations in " << duration.count() << " μs\n";
    std::cout << "   Performance: " << (500000.0 * 1000000.0) / duration.count() << " ops/sec\n";
    
    std::cout << "\n✅ Example completed successfully!\n";
    std::cout << "\nAPI Summary:\n";
    std::cout << "• get(key) noexcept          - Maximum performance\n";
    std::cout << "• getOrThrow(key)            - Exception-based error handling\n";
    std::cout << "• getOrDefault(key, default) - Safe access with fallback\n";
    std::cout << "• put(key, value) noexcept   - High-performance insertion\n";
    std::cout << "• contains(key) noexcept     - Fast existence check\n";
    
    return 0;
}