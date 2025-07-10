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

template<typename Key, typename Value, size_t MaxSize, typename Hash = std::hash<Key>>
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
        inline void addToHead(Node* node) {
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
        inline void remove(Node* node) {
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
        inline bool empty() const { return size == 0; }
    };
    
    int minFrequency_;
    
    // Fixed-size memory pool for maximum performance
    std::array<Node, MaxSize> nodePool_;
    std::array<int, MaxSize> freeNodes_;
    int poolSize_;
    int freeCount_;
    
    std::unordered_map<Key, Node*, Hash> keyToNode_;
    std::unordered_map<int, FrequencyList> frequencyToList_;
    
private:
    // OPTIMIZATION: Force inlining of allocation functions (hot path)
    inline Node* allocateNode(const Key& key, const Value& value, int frequency) {
        if (freeCount_ > 0) [[likely]] {  // OPTIMIZATION: Branch prediction hint
            // Reuse freed slot
            --freeCount_;
            int idx = freeNodes_[freeCount_];
            Node* node = &nodePool_[idx];
            *node = Node(key, value, frequency);
            return node;
        }
        
        // OPTIMIZATION: Template specialization opportunity for compile-time bounds check
        if constexpr (MaxSize > 0) {
            if (poolSize_ >= static_cast<int>(MaxSize)) [[unlikely]] {  // OPTIMIZATION: Branch prediction hint
                throw std::runtime_error("Node pool exhausted - increase MaxSize template parameter");
            }
        }
        
        // Use next available slot in fixed array
        Node* node = &nodePool_[poolSize_];
        *node = Node(key, value, frequency);
        poolSize_++;
        return node;
    }
    
    // OPTIMIZATION: Force inlining of deallocation (hot path)
    inline void deallocateNode(Node* node) {
        // OPTIMIZATION: Pointer arithmetic optimization with explicit bounds
        int idx = static_cast<int>(node - &nodePool_[0]);
        
        // OPTIMIZATION: Template-based bounds checking for better optimization
        if constexpr (MaxSize > 0) {
            assert(idx >= 0 && idx < static_cast<int>(MaxSize) && "Invalid node pointer");
            // OPTIMIZATION: Remove unreachable return after assert in release builds
        }
        
        // Add to free list
        freeNodes_[freeCount_] = idx;
        ++freeCount_;
    }
    
    // OPTIMIZATION: Force inlining of frequency update (most critical function)
    inline void updateFrequency(Node* node) {
        int oldFreq = node->frequency;
        int newFreq = oldFreq + 1;
        
        // Remove from old frequency list
        auto oldIt = frequencyToList_.find(oldFreq);
        if (oldIt != frequencyToList_.end()) [[likely]] {
            oldIt->second.remove(node);
            // Clean up empty frequency lists to prevent memory waste
            if (oldIt->second.empty()) [[unlikely]] {
                frequencyToList_.erase(oldIt);
            }
        }
        
        // Update frequency and add to new list
        node->frequency = newFreq;
        frequencyToList_[newFreq].addToHead(node);
        
        // OPTIMIZATION: Update minimum frequency
        if (oldFreq == minFrequency_ && frequencyToList_.find(minFrequency_) == frequencyToList_.end()) [[unlikely]] {
            minFrequency_++;
        }
    }
    
public:
    LFUCache() 
        : minFrequency_(0), poolSize_(0), freeCount_(0) {
        
        // OPTIMIZATION: Template-based compile-time validation
        static_assert(MaxSize > 0, "MaxSize must be positive");
        
        // OPTIMIZATION: Reserve space for common frequencies to avoid rehashing
        frequencyToList_.reserve(std::max(MIN_FREQUENCY_SIZE, 
                                        MaxSize / INITIAL_SIZE_MULTIPLIER));
    }
    
    // OPTIMIZATION: Hot path version - no exceptions for maximum performance
    inline Value get(const Key& key) noexcept {
        auto it = keyToNode_.find(key);
        if (it == keyToNode_.end()) [[unlikely]] {  // OPTIMIZATION: Branch prediction hint
            return Value{};  // Return default-constructed value for missing keys
        }
        
        Node* node = it->second;
        updateFrequency(node);
        return node->value;
    }
    
    // Exception-throwing version for when you need error handling
    inline Value getOrThrow(const Key& key) {
        auto it = keyToNode_.find(key);
        if (it == keyToNode_.end()) [[unlikely]] {  // OPTIMIZATION: Branch prediction hint
            throw std::runtime_error("Key not found");
        }
        
        Node* node = it->second;
        updateFrequency(node);
        return node->value;
    }
    
    // OPTIMIZATION: Force inlining of getOrDefault function (hot path) - already noexcept
    inline Value getOrDefault(const Key& key, const Value& defaultValue) noexcept {
        auto it = keyToNode_.find(key);
        if (it == keyToNode_.end()) [[unlikely]] {  // OPTIMIZATION: Branch prediction hint
            return defaultValue;
        }
        
        Node* node = it->second;
        updateFrequency(node);
        return node->value;
    }
    
    // OPTIMIZATION: Force inlining of contains function (hot path) - noexcept for performance
    inline bool contains(const Key& key) const noexcept {
        return keyToNode_.find(key) != keyToNode_.end();
    }
    
    // OPTIMIZATION: Hot path put - noexcept for maximum performance
    void put(const Key& key, const Value& value) noexcept {
        auto it = keyToNode_.find(key);
        if (it != keyToNode_.end()) [[likely]] {  // OPTIMIZATION: Branch prediction hint - cache updates are common
            // Update existing key
            Node* node = it->second;
            node->value = value;
            updateFrequency(node);
            return;
        }
        
        // Add new key - check capacity
        if (keyToNode_.size() >= MaxSize) [[likely]] {  // OPTIMIZATION: Branch prediction hint
            // Remove least frequently used item
            auto minFreqIt = frequencyToList_.find(minFrequency_);
            if (minFreqIt != frequencyToList_.end() && !minFreqIt->second.empty()) [[likely]] {
                Node* lru = minFreqIt->second.tail;
                minFreqIt->second.remove(lru);
                keyToNode_.erase(lru->key);
                deallocateNode(lru);
                // Clean up empty frequency list
                if (minFreqIt->second.empty()) [[unlikely]] {
                    frequencyToList_.erase(minFreqIt);
                }
            }
        }
        
        // Add new node
        Node* newNode = allocateNode(key, value, 1);
        keyToNode_[key] = newNode;

        frequencyToList_[1].addToHead(newNode);
        minFrequency_ = 1;
    }
    
    // OPTIMIZATION: Force inlining of simple getters - noexcept for performance
    inline int size() const noexcept {
        return static_cast<int>(keyToNode_.size());
    }
    
    inline constexpr size_t capacity() const noexcept {
        return MaxSize;
    }
    
    void clear() noexcept {
        keyToNode_.clear();
        frequencyToList_.clear();
        
        // OPTIMIZATION: Loop optimization - use standard algorithm instead of manual loop
        if (poolSize_ > 0) {
            std::iota(freeNodes_.begin(), freeNodes_.begin() + poolSize_, 0);
        }
        freeCount_ = poolSize_;
        poolSize_ = 0;
        minFrequency_ = 0;
    }
    
    // Debug function with optimization hints
    void printState() const {
        std::cout << "Cache State (size=" << size() << ", capacity=" << MaxSize << "):\n";
        for (const auto& [freq, list] : frequencyToList_) {
            if (!list.empty()) {
                std::cout << "  Freq " << freq << ": ";
                Node* current = list.head;
                while (current) [[likely]] {  // OPTIMIZATION: Branch prediction hint
                    std::cout << "(" << current->key << "," << current->value << ") ";
                    current = current->next;
                }
                std::cout << "\n";
            }
        }
        std::cout << "  Min frequency: " << minFrequency_ << "\n";
    }
};

#endif // LFU_CACHE_H