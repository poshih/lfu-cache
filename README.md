!!!!!!!!!!!AI Assisted Generated Text below Read it at your own peril!!!!!!!!!!!!! I would say it's about 98% correct - Po

# High-Performance LFU Cache

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Performance](https://img.shields.io/badge/Performance-23M%2B%20ops%2Fsec-green.svg)](https://github.com/poshihtsang/lfu-cache)

A highly optimized **Least Frequently Used (LFU) cache** implementation in C++20 featuring a hybrid API design that combines maximum performance with flexible error handling. This implementation applies comprehensive static program analysis optimizations for enterprise-grade performance requirements.

## 🚀 Key Features

- **True O(1) Operations**: Guaranteed O(1) average and worst-case complexity for get() and put()
- **High Performance**: 23M+ operations/second on modern hardware (~43ns per operation)
- **Hybrid API**: Choose between noexcept performance or exception-based error handling
- **Zero Allocation**: Fixed-size memory pools eliminate heap allocation during operation
- **Cache Optimized**: 64-byte aligned nodes for optimal CPU cache line utilization
- **Template Based**: Generic design works with any hashable key and copyable value types

## 🎮 Perfect for Game Development

**Why LFU Cache is Ideal for Games:**
- **✨ Known Max Assets**: Game assets (textures, sounds, models) have predictable maximums
- **🎲 Unpredictable Access**: Players access content in unpredictable patterns
- **⚡ Frequency Matters**: Popular assets should stay cached, rare ones can be evicted
- **🚀 Performance Critical**: Zero allocation during gameplay, no memory fragmentation
- **🎁 Event-Driven**: Perfect for infrequent but important events (achievements, rare items)

**Game Use Cases:**
```cpp
// Audio samples - common sounds stay, rare sounds evicted
LFUCache<SoundId, AudioClip, 1000> audioCache;

// UI elements - frequent menus cached, rare dialogs evicted  
LFUCache<UIElementId, UITexture, 500> uiCache;

// Game objects - active items cached, inactive evicted
LFUCache<ItemId, ItemData, 2000> itemCache;
```

## 📊 Performance Characteristics

| Metric | Value |
|--------|-------|
| **Throughput** | 23M+ operations/second |
| **Latency** | ~43ns average per operation |
| **Complexity** | O(1) guaranteed for all operations |
| **Memory** | 64 bytes per node (cache-line aligned) |
| **Allocation** | Zero heap allocation during operation |

## 🔧 Static Optimizations Applied

- ✅ **Dead code elimination** for unreachable branches
- ✅ **Aggressive function inlining** for hot paths
- ✅ **Constant folding and propagation** at compile time
- ✅ **Branch prediction hints** with `[[likely]]`/`[[unlikely]]`
- ✅ **Strength reduction** optimizations for arithmetic
- ✅ **Memory layout optimization** with cache-line alignment
- ✅ **Loop optimizations** using standard algorithms
- ✅ **Template specialization** to reduce code bloat

## 📖 Usage Examples

### Basic Usage

```cpp
#include "lfu_cache.h"

// Create cache: <Key, Value, MaxSize> - MaxSize is the capacity
LFUCache<int, std::string, 1000> cache; // Capacity is 1000 (compile-time)

// Hot path - maximum performance (noexcept)
cache.put(1, "value");
auto value = cache.get(1);  // Returns "" if not found
auto safe = cache.getOrDefault(2, "fallback");  // Custom fallback
```

### Error Handling

```cpp
// When you need strict validation
try {
    auto value = cache.getOrThrow(999);  // Throws std::runtime_error if not found
} catch (const std::runtime_error& e) {
    std::cout << "Key not found: " << e.what() << std::endl;
}
```

### Performance Critical Code

```cpp
// Ultra-high performance loop (all noexcept)
for (int i = 0; i < 1000000; ++i) {
    if (cache.contains(key)) {      // noexcept check
        process(cache.get(key));    // noexcept access
    }
}
```

## 🏗️ Hybrid API Design

| Function | Exception Safety | Use Case |
|----------|------------------|----------|
| `get(key)` | `noexcept` | **Hot paths**, maximum performance |
| `getOrThrow(key)` | **Throws** | Input validation, error handling |
| `getOrDefault(key, default)` | `noexcept` | Safe access with fallbacks |
| `put(key, value)` | `noexcept` | High-performance insertion |
| `contains(key)` | `noexcept` | Existence checks |
| `size()`, `capacity()` | `noexcept` | Metadata access (capacity is compile-time) |

## 🔧 Template Parameters

```cpp
template<typename Key, typename Value, size_t MaxSize, typename Hash = std::hash<Key>>
class LFUCache;
```

- **`Key`**: Key type (must be hashable and equality comparable)
- **`Value`**: Value type (must be default constructible for `get()` noexcept)
- **`MaxSize`**: Maximum number of elements (compile-time capacity)
- **`Hash`**: Custom hash function (defaults to `std::hash<Key>`)

## 💾 Memory Requirements

- **Node size**: 64 bytes per element (cache-line aligned)
- **Total memory**: `MaxSize * 64 bytes + hash table overhead`
- **Example**: MaxSize=1000 ≈ 64KB + hash table

## 🧵 Thread Safety

This implementation is **not thread-safe** by design for maximum performance. For concurrent access, wrap with external synchronization:

```cpp
#include <mutex>

std::mutex cache_mutex;
LFUCache<int, std::string, 1000> cache;

// Thread-safe wrapper example
auto thread_safe_get = [&](int key) -> std::string {
    std::lock_guard<std::mutex> lock(cache_mutex);
    return cache.get(key);
};
```

## 🏗️ Building and Testing

### Requirements

- **C++20** compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- **CMake** 3.10+ (optional)

### Compilation

```bash
# Simple compilation
g++ -std=c++20 -O3 -march=native -DNDEBUG your_code.cpp

# With all optimizations
g++ -std=c++20 -O3 -march=native -DNDEBUG -flto -ffast-math your_code.cpp
```

### Running Tests

```bash
# Compile and run the test suite
g++ -std=c++20 -O3 -march=native -DNDEBUG -o test_lfu test_optimized_lfu.cpp
./test_lfu

# Run performance benchmark
g++ -std=c++20 -O3 -march=native -DNDEBUG -o benchmark hybrid_benchmark.cpp
./benchmark
```

## 📈 Benchmarks

Performance measured on Intel i7-9750H @ 2.60GHz:

```
=== PERFORMANCE RESULTS ===
Operations: 2,000,000
Cache size: 1,000

Hybrid noexcept get():    22,969,848 ops/sec
Exception-based get():    22,738,297 ops/sec
Performance improvement:  1.01% faster (noexcept)
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Po Shih Tsang**
- GitHub: [@poshihtsang](https://github.com/poshihtsang)

---

⭐ **Star this repository if it helped you build faster applications!**