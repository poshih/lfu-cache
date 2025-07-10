/*
 * Hybrid API Performance Benchmark
 * 
 * This program specifically measures the performance improvement
 * of the noexcept hybrid approach vs exception-based approach.
 */

#include "lfu_cache.h"
#include <chrono>
#include <random>
#include <iostream>
#include <iomanip>

// Version of optimized cache that still throws exceptions (for comparison)
template<typename Key, typename Value, size_t MaxSize, typename Hash = std::hash<Key>>
class LFUCacheWithExceptions {
public:
    static constexpr size_t MIN_FREQUENCY_SIZE = 16;
    static constexpr size_t INITIAL_SIZE_MULTIPLIER = 10;
    static constexpr size_t GROWTH_FACTOR = 2;
    
    struct alignas(64) Node {
        int frequency;
        Node* prev;
        Node* next;
        Key key;
        Value value;
        
        Node() : frequency(0), prev(nullptr), next(nullptr) {}
        Node(const Key& k, const Value& v, int f) 
            : frequency(f), prev(nullptr), next(nullptr), key(k), value(v) {}
    };
    
    struct FrequencyList {
        Node* head;
        Node* tail;
        int size;
        
        FrequencyList() : head(nullptr), tail(nullptr), size(0) {}
        
        inline void addToHead(Node* node) {
            node->prev = nullptr;
            node->next = head;
            if (head) [[likely]] {
                head->prev = node;
            }
            head = node;
            if (!tail) [[unlikely]] {
                tail = node;
            }
            size++;
        }
        
        inline void remove(Node* node) {
            if (node->prev) [[likely]] {
                node->prev->next = node->next;
            } else {
                head = node->next;
            }
            if (node->next) [[likely]] {
                node->next->prev = node->prev;
            } else {
                tail = node->prev;
            }
            node->prev = node->next = nullptr;
            size--;
        }
        
        inline bool empty() const { return size == 0; }
    };
    
private:
    int minFrequency_;
    std::array<Node, MaxSize> nodePool_;
    std::array<int, MaxSize> freeNodes_;
    int poolSize_;
    int freeCount_;
    std::unordered_map<Key, Node*, Hash> keyToNode_;
    std::vector<FrequencyList> frequencyToList_;
    
    inline Node* allocateNode(const Key& key, const Value& value, int frequency) {
        if (freeCount_ > 0) [[likely]] {
            --freeCount_;
            int idx = freeNodes_[freeCount_];
            Node* node = &nodePool_[idx];
            *node = Node(key, value, frequency);
            return node;
        }
        
        if constexpr (MaxSize > 0) {
            if (poolSize_ >= static_cast<int>(MaxSize)) [[unlikely]] {
                throw std::runtime_error("Node pool exhausted");
            }
        }
        
        Node* node = &nodePool_[poolSize_];
        *node = Node(key, value, frequency);
        poolSize_++;
        return node;
    }
    
    inline void deallocateNode(Node* node) {
        int idx = static_cast<int>(node - &nodePool_[0]);
        if constexpr (MaxSize > 0) {
            assert(idx >= 0 && idx < static_cast<int>(MaxSize) && "Invalid node pointer");
        }
        freeNodes_[freeCount_] = idx;
        ++freeCount_;
    }
    
    inline void updateFrequency(Node* node) {
        int oldFreq = node->frequency;
        int newFreq = oldFreq + 1;
        
        if (newFreq >= static_cast<int>(frequencyToList_.size())) [[unlikely]] {
            size_t currentSize = frequencyToList_.size();
            size_t newSize = std::max(static_cast<size_t>(newFreq + 1), 
                                     currentSize * GROWTH_FACTOR);
            frequencyToList_.resize(newSize);
        }
        
        frequencyToList_[oldFreq].remove(node);
        node->frequency = newFreq;
        frequencyToList_[newFreq].addToHead(node);
        
        if (frequencyToList_[minFrequency_].empty()) [[unlikely]] {
            minFrequency_++;
        }
    }
    
public:
    LFUCacheWithExceptions() 
        : minFrequency_(0), poolSize_(0), freeCount_(0) {
        
        static_assert(MaxSize > 0, "MaxSize must be positive");
        
        size_t initialSize = std::max(MIN_FREQUENCY_SIZE, 
                                     MaxSize / INITIAL_SIZE_MULTIPLIER);
        frequencyToList_.resize(initialSize);
    }
    
    // THROWS EXCEPTIONS (old approach)
    inline Value get(const Key& key) {
        auto it = keyToNode_.find(key);
        if (it == keyToNode_.end()) [[unlikely]] {
            throw std::runtime_error("Key not found");
        }
        
        Node* node = it->second;
        updateFrequency(node);
        return node->value;
    }
    
    inline bool contains(const Key& key) const {
        return keyToNode_.find(key) != keyToNode_.end();
    }
    
    void put(const Key& key, const Value& value) {
        auto it = keyToNode_.find(key);
        if (it != keyToNode_.end()) [[likely]] {
            Node* node = it->second;
            node->value = value;
            updateFrequency(node);
            return;
        }
        
        if (keyToNode_.size() >= MaxSize) [[likely]] {
            auto& minFreqList = frequencyToList_[minFrequency_];
            if (!minFreqList.empty()) [[likely]] {
                Node* lru = minFreqList.tail;
                minFreqList.remove(lru);
                keyToNode_.erase(lru->key);
                deallocateNode(lru);
            }
        }
        
        if (frequencyToList_.size() <= 1) [[unlikely]] {
            size_t newSize = std::max(MIN_FREQUENCY_SIZE, 
                                     frequencyToList_.size() * GROWTH_FACTOR);
            frequencyToList_.resize(newSize);
        }
        
        Node* newNode = allocateNode(key, value, 1);
        keyToNode_[key] = newNode;
        frequencyToList_[1].addToHead(newNode);
        minFrequency_ = 1;
    }
    
    inline int size() const {
        return static_cast<int>(keyToNode_.size());
    }
};

// Benchmark function
template<typename CacheType>
double benchmarkCache(const std::string& name, bool useGetOrThrow = false) {
    const int NUM_OPERATIONS = 2000000;  // Increased for better measurement
    const int CACHE_SIZE = 1000;
    const int ITERATIONS = 3;
    
    std::vector<double> times;
    times.reserve(ITERATIONS);
    
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        CacheType cache;
        
        std::mt19937 gen(42 + iter);
        std::uniform_int_distribution<> keyDist(1, CACHE_SIZE * 2);
        std::uniform_int_distribution<> opDist(0, 100);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        volatile int dummy = 0;
        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            int key = keyDist(gen);
            int op = opDist(gen);
            
            if (op < 70) {  // 70% gets
                if (cache.contains(key)) {
                    if constexpr (std::is_same_v<CacheType, LFUCache<int, int, 4000>>) {
                        if (useGetOrThrow) {
                            dummy += cache.getOrThrow(key);
                        } else {
                            dummy += cache.get(key);  // noexcept version
                        }
                    } else {
                        try {
                            dummy += cache.get(key);  // exception version
                        } catch (...) {
                            // Should never happen since we check contains()
                        }
                    }
                }
            } else {  // 30% puts
                cache.put(key, key * 10);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        times.push_back(duration.count());
    }
    
    // Calculate average
    double sum = 0;
    for (double time : times) {
        sum += time;
    }
    double average = sum / ITERATIONS;
    
    std::cout << name << ":\n";
    std::cout << "  Average time: " << std::fixed << std::setprecision(2) << average << " μs\n";
    std::cout << "  Average ops/sec: " << std::fixed << std::setprecision(0) 
              << (NUM_OPERATIONS * 1000000.0) / average << "\n\n";
    
    return average;
}

int main() {
    std::cout << "=== HYBRID API PERFORMANCE BENCHMARK ===\n";
    std::cout << "Operations per test: 2,000,000\n";
    std::cout << "Iterations: 3 (averaged)\n";
    std::cout << "Cache size: 1,000\n\n";
    
    // Benchmark exception-based approach
    double timeWithExceptions = benchmarkCache<LFUCacheWithExceptions<int, int, 4000>>("Exception-based get()");
    
    // Benchmark noexcept approach  
    double timeNoExcept = benchmarkCache<LFUCache<int, int, 4000>>("Hybrid noexcept get()", false);
    
    // Benchmark throwing version of hybrid
    double timeGetOrThrow = benchmarkCache<LFUCache<int, int, 4000>>("Hybrid getOrThrow()", true);
    
    // Results analysis
    std::cout << "=== PERFORMANCE ANALYSIS ===\n";
    
    double improvementNoExcept = ((timeWithExceptions - timeNoExcept) / timeWithExceptions) * 100;
    double improvementOverGetOrThrow = ((timeGetOrThrow - timeNoExcept) / timeGetOrThrow) * 100;
    
    std::cout << "🚀 noexcept get() vs exception get(): " << std::fixed << std::setprecision(2) 
              << improvementNoExcept << "% faster\n";
    std::cout << "🚀 noexcept get() vs getOrThrow(): " << std::fixed << std::setprecision(2) 
              << improvementOverGetOrThrow << "% faster\n";
    
    std::cout << "\nSpeedup ratios:\n";
    std::cout << "  noexcept vs exceptions: " << std::fixed << std::setprecision(3) 
              << timeWithExceptions / timeNoExcept << "x\n";
    std::cout << "  noexcept vs getOrThrow: " << std::fixed << std::setprecision(3) 
              << timeGetOrThrow / timeNoExcept << "x\n";
    
    std::cout << "\n=== RECOMMENDATION ===\n";
    if (improvementNoExcept > 2.0) {
        std::cout << "✅ Hybrid approach provides significant performance benefit!\n";
        std::cout << "   Use get() for hot paths, getOrThrow() for error handling\n";
    } else {
        std::cout << "⚖️  Performance improvement is modest\n";
        std::cout << "   Hybrid approach still provides better API flexibility\n";
    }
    
    return 0;
}