/*
 * High-Performance LFU Cache with Hybrid API and Static Optimizations
 * 
 * MIT License - Copyright (c) 2024 Po Shih Tsang
 * 
 * Author: Po Shih Tsang
 * GitHub: https://github.com/poshih/lfu-cache/
 * 
 * DESCRIPTION:
 * A static Fixed Size LFUCache, good for use when you already know the maximum size of the cache.
  */

#ifndef LFU_CACHE_H
#define LFU_CACHE_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <array>
#include <cassert>
#include <chrono>
#include <random>
#include <algorithm>
#include <numeric>

template<typename Key, typename Value, size_t MAX_SIZE, typename Hash = std::hash<Key>>
class LFUCache {
public:
    // OPTIMIZATION: Constant folding - compile-time constants
    static constexpr size_t MIN_FREQUENCY_SIZE = 16;
    static constexpr size_t INITIAL_SIZE_MULTIPLIER = 10;
    static constexpr size_t GROWTH_FACTOR = 2;
    
    struct Node {
        // Hot fields first (accessed most frequently)
        int frequency;         // Most accessed field
        Node* prev;           // Pointer fields together  
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
        
        // OPTIMIZATION: Force inlining of critical path functions
        inline void AddToHead(Node* node) {
            node->prev = nullptr;
            node->next = head;
            if (head) [[likely]] {  // OPTIMIZATION: Branch prediction hint
                head->prev = node;
            }
            head = node;
            if (!tail) [[unlikely]] {  // OPTIMIZATION: Branch prediction hint
                tail = node;
            }
            size++;
        }
        
        // OPTIMIZATION: Force inlining of critical path functions
        inline void Remove(Node* node) {
            if (node->prev) [[likely]] {  // OPTIMIZATION: Branch prediction hint
                node->prev->next = node->next;
            } else {
                head = node->next;
            }
            if (node->next) [[likely]] {  // OPTIMIZATION: Branch prediction hint
                node->next->prev = node->prev;
            } else {
                tail = node->prev;
            }
            node->prev = node->next = nullptr;
            size--;
        }
        
        // OPTIMIZATION: Force inlining of simple getter
        inline bool Empty() const { return size == 0; }
    };
    
    int minFrequency;
    
    // Fixed-size memory pool for maximum performance
    std::array<Node, MAX_SIZE> nodePool;
    std::array<int, MAX_SIZE> freeNodes;
    int poolSize;
    int freeCount;
    
    std::unordered_map<Key, Node*, Hash> keyToNode;
    std::unordered_map<int, FrequencyList> frequencyToList;
    
private:
    // OPTIMIZATION: Force inlining of allocation functions (hot path)
    inline Node* allocateNode(const Key& key, const Value& value, int frequency) {
        if (freeCount > 0) [[likely]] {  // OPTIMIZATION: Branch prediction hint
            // Reuse freed slot
            --freeCount;
            int idx = freeNodes[freeCount];
            Node* node = &nodePool[idx];
            *node = Node(key, value, frequency);
            return node;
        }
        
        // OPTIMIZATION: Template specialization opportunity for compile-time bounds check
        if constexpr (MAX_SIZE > 0) {
            if (poolSize >= static_cast<int>(MAX_SIZE)) [[unlikely]] {  // OPTIMIZATION: Branch prediction hint
                throw std::runtime_error("Node pool exhausted - increase MAX_SIZE template parameter");
            }
        }
        
        // Use next available slot in fixed array
        Node* node = &nodePool[poolSize];
        *node = Node(key, value, frequency);
        poolSize++;
        return node;
    }
    
    // OPTIMIZATION: Force inlining of deallocation (hot path)
    inline void deallocateNode(Node* node) {
        // OPTIMIZATION: Pointer arithmetic optimization with explicit bounds
        int idx = static_cast<int>(node - &nodePool[0]);
        
        // OPTIMIZATION: Template-based bounds checking for better optimization
        if constexpr (MAX_SIZE > 0) {
            assert(idx >= 0 && idx < static_cast<int>(MAX_SIZE) && "Invalid node pointer");
            // OPTIMIZATION: Remove unreachable return after assert in release builds
        }
        
        // Add to free list
        freeNodes[freeCount] = idx;
        ++freeCount;
    }
    
    // OPTIMIZATION: Force inlining of frequency update (most critical function)
    inline void updateFrequency(Node* node) {
        int oldFreq = node->frequency;
        int newFreq = oldFreq + 1;
        
        // Remove from old frequency list
        auto oldIt = frequencyToList.find(oldFreq);
        if (oldIt != frequencyToList.end()) [[likely]] {
            oldIt->second.Remove(node);
            // Clean up empty frequency lists to prevent memory waste
            if (oldIt->second.Empty()) [[unlikely]] {
                frequencyToList.erase(oldIt);
            }
        }
        
        // Update frequency and add to new list
        node->frequency = newFreq;
        frequencyToList[newFreq].AddToHead(node);
        
        // OPTIMIZATION: Update minimum frequency
        if (oldFreq == minFrequency && frequencyToList.find(minFrequency) == frequencyToList.end()) [[unlikely]] {
            minFrequency++;
        }
    }
    
public:
    LFUCache() 
        : minFrequency(0), poolSize(0), freeCount(0) {
        
        // OPTIMIZATION: Template-based compile-time validation
        static_assert(MAX_SIZE > 0, "MAX_SIZE must be positive");
        
        // OPTIMIZATION: Reserve space for common frequencies to avoid rehashing
        frequencyToList.reserve(std::max(MIN_FREQUENCY_SIZE, 
                                        MAX_SIZE / INITIAL_SIZE_MULTIPLIER));
    }
    
    // OPTIMIZATION: Hot path version - no exceptions for maximum performance
    inline Value Get(const Key& key) noexcept {
        auto it = keyToNode.find(key);
        if (it == keyToNode.end()) [[unlikely]] {  // OPTIMIZATION: Branch prediction hint
            return Value{};  // Return default-constructed value for missing keys
        }
        
        Node* node = it->second;
        updateFrequency(node);
        return node->value;
    }
    
    // Exception-throwing version for when you need error handling
    inline Value GetOrThrow(const Key& key) {
        auto it = keyToNode.find(key);
        if (it == keyToNode.end()) [[unlikely]] {  // OPTIMIZATION: Branch prediction hint
            throw std::runtime_error("Key not found");
        }
        
        Node* node = it->second;
        updateFrequency(node);
        return node->value;
    }
    
    // OPTIMIZATION: Force inlining of getOrDefault function (hot path) - already noexcept
    inline Value GetOrDefault(const Key& key, const Value& defaultValue) noexcept {
        auto it = keyToNode.find(key);
        if (it == keyToNode.end()) [[unlikely]] {  // OPTIMIZATION: Branch prediction hint
            return defaultValue;
        }
        
        Node* node = it->second;
        updateFrequency(node);
        return node->value;
    }
    
    // OPTIMIZATION: Force inlining of contains function (hot path) - noexcept for performance
    inline bool Contains(const Key& key) const noexcept {
        return keyToNode.find(key) != keyToNode.end();
    }
    
    // OPTIMIZATION: Hot path put - noexcept for maximum performance
    void Put(const Key& key, const Value& value) noexcept {
        auto it = keyToNode.find(key);
        if (it != keyToNode.end()) [[likely]] {  // OPTIMIZATION: Branch prediction hint - cache updates are common
            // Update existing key
            Node* node = it->second;
            node->value = value;
            updateFrequency(node);
            return;
        }
        
        // Add new key - check capacity
        if (keyToNode.size() >= MAX_SIZE) [[likely]] {  // OPTIMIZATION: Branch prediction hint
            // Remove least frequently used item
            auto minFreqIt = frequencyToList.find(minFrequency);
            if (minFreqIt != frequencyToList.end() && !minFreqIt->second.Empty()) [[likely]] {
                Node* lru = minFreqIt->second.tail;
                minFreqIt->second.Remove(lru);
                keyToNode.erase(lru->key);
                deallocateNode(lru);
                // Clean up empty frequency list
                if (minFreqIt->second.Empty()) [[unlikely]] {
                    frequencyToList.erase(minFreqIt);
                }
            }
        }
        
        // Add new node
        Node* newNode = allocateNode(key, value, 1);
        keyToNode[key] = newNode;

        frequencyToList[1].AddToHead(newNode);
        minFrequency = 1;
    }
    
    // OPTIMIZATION: Force inlining of simple getters - noexcept for performance
    inline int Size() const noexcept {
        return static_cast<int>(keyToNode.size());
    }
    
    inline constexpr size_t Capacity() const noexcept {
        return MAX_SIZE;
    }
    
    void Clear() noexcept {
        keyToNode.clear();
        frequencyToList.clear();
        
        // OPTIMIZATION: Loop optimization - use standard algorithm instead of manual loop
        if (poolSize > 0) {
            std::iota(freeNodes.begin(), freeNodes.begin() + poolSize, 0);
        }
        freeCount = poolSize;
        poolSize = 0;
        minFrequency = 0;
    }
    
    // Debug function with optimization hints
    void PrintState() const {
        std::cout << "Cache State (size=" << Size() << ", capacity=" << MAX_SIZE << "):\n";
        for (const auto& [freq, list] : frequencyToList) {
            if (!list.Empty()) {
                std::cout << "  Freq " << freq << ": ";
                Node* current = list.head;
                while (current) [[likely]] {  // OPTIMIZATION: Branch prediction hint
                    std::cout << "(" << current->key << "," << current->value << ") ";
                    current = current->next;
                }
                std::cout << "\n";
            }
        }
        std::cout << "  Min frequency: " << minFrequency << "\n";
    }
};

#endif // LFU_CACHE_H