/*
 * Hybrid API Usage Example
 * 
 * This demonstrates how to use the hybrid noexcept/exception API
 * for different scenarios in the optimized LFU cache.
 */

#include "lfu_cache.h"
#include <iostream>
#include <string>

void demonstrateHotPath() {
    std::cout << "=== HOT PATH EXAMPLE (noexcept for maximum performance) ===\n";
    
    // Create cache with 100 capacity, 500 max size
    LFUCache<int, std::string, 500> cache;
    
    // Hot path operations - all noexcept
    cache.put(1, "user1");
    cache.put(2, "user2");
    cache.put(3, "user3");
    
    // High-performance access - no exceptions thrown
    if (cache.contains(1)) {
        auto value = cache.get(1);  // Returns "user1", noexcept
        std::cout << "User 1: " << value << " (noexcept access)\n";
    }
    
    // Safe access with fallback - no exceptions
    auto value = cache.getOrDefault(999, "guest");  // Returns "guest"
    std::cout << "User 999: " << value << " (safe fallback)\n";
    
    // Missing key with noexcept get() - returns default value
    auto missing = cache.get(404);  // Returns "", no exception
    std::cout << "Missing key returns: '" << missing << "' (empty string)\n";
    
    std::cout << "Cache size: " << cache.size() << " (noexcept)\n\n";
}

void demonstrateErrorHandling() {
    std::cout << "=== ERROR HANDLING EXAMPLE (exceptions for validation) ===\n";
    
    LFUCache<std::string, int, 100> cache;
    
    cache.put("score1", 100);
    cache.put("score2", 200);
    
    // When you need strict error handling
    try {
        auto score = cache.getOrThrow("score1");  // Throws if not found
        std::cout << "Score 1: " << score << " (validated access)\n";
        
        auto missing = cache.getOrThrow("score999");  // Will throw
        std::cout << "This won't print\n";
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected exception: " << e.what() << "\n";
    }
    
    // Mixed usage - performance critical path with error handling
    for (const std::string& key : {"score1", "score2", "missing"}) {
        if (cache.contains(key)) {
            // Hot path - noexcept
            auto value = cache.get(key);
            std::cout << key << ": " << value << " (fast path)\n";
        } else {
            std::cout << key << ": not found (checked first)\n";
        }
    }
    std::cout << "\n";
}

void demonstratePerformanceCritical() {
    std::cout << "=== PERFORMANCE CRITICAL EXAMPLE ===\n";
    
    LFUCache<int, double, 1000> cache;
    
    // Simulate high-frequency trading data
    for (int i = 1; i <= 100; ++i) {
        cache.put(i, i * 3.14159);
    }
    
    const int HOT_KEYS[] = {1, 5, 10, 25, 50};
    
    // Performance critical loop - all noexcept
    double sum = 0.0;
    for (int iter = 0; iter < 10000; ++iter) {
        for (int key : HOT_KEYS) {
            // Ultra-fast access - no exception handling overhead
            sum += cache.get(key);  // noexcept, maximum performance
        }
    }
    
    std::cout << "Processed 50,000 cache accesses (noexcept)\n";
    std::cout << "Total sum: " << sum << "\n";
    std::cout << "Cache efficiency: " << cache.size() << "/" << cache.capacity() << "\n\n";
}

void demonstrateMixedScenario() {
    std::cout << "=== MIXED SCENARIO EXAMPLE ===\n";
    
    LFUCache<std::string, std::string, 200> cache;
    
    // Setup data
    cache.put("config.timeout", "30");
    cache.put("config.retries", "3");
    cache.put("config.host", "localhost");
    
    // Configuration reader - needs validation
    auto readConfig = [&cache](const std::string& key) -> std::string {
        try {
            return cache.getOrThrow(key);  // Strict validation
        } catch (const std::runtime_error&) {
            throw std::runtime_error("Missing required config: " + key);
        }
    };
    
    // Hot path accessor - performance critical
    auto quickLookup = [&cache](const std::string& key, const std::string& defaultVal) -> std::string {
        return cache.getOrDefault(key, defaultVal);  // noexcept, fast
    };
    
    try {
        // Critical config - must exist
        std::cout << "Timeout: " << readConfig("config.timeout") << "s\n";
        std::cout << "Retries: " << readConfig("config.retries") << "\n";
        
        // Optional config - use defaults
        std::cout << "Debug: " << quickLookup("config.debug", "false") << "\n";
        std::cout << "Port: " << quickLookup("config.port", "8080") << "\n";
        
        // This will throw
        std::cout << readConfig("config.missing") << "\n";
        
    } catch (const std::runtime_error& e) {
        std::cout << "Config error: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "Hybrid API LFU Cache Examples\n";
    std::cout << "==============================\n\n";
    
    demonstrateHotPath();
    demonstrateErrorHandling();
    demonstratePerformanceCritical();
    demonstrateMixedScenario();
    
    std::cout << "\n=== API SUMMARY ===\n";
    std::cout << "✅ get(key) noexcept          - Maximum performance, returns default for missing\n";
    std::cout << "✅ getOrThrow(key)            - Exception-based error handling\n";
    std::cout << "✅ getOrDefault(key, default) - Safe access with custom fallback\n";
    std::cout << "✅ contains(key) noexcept     - Fast existence check\n";
    std::cout << "✅ put(key, value) noexcept   - High-performance insertion\n";
    std::cout << "✅ Constructor exceptions     - Input validation at creation\n";
    
    return 0;
}