# Examples

This directory contains various examples demonstrating the High-Performance LFU Cache usage.

## 📁 Files Overview

### **simple_example.cpp**
Basic usage demonstration showing:
- Cache creation and basic operations
- Hybrid API usage (noexcept vs exceptions)
- LFU eviction behavior
- Performance demonstration

**Compile & Run:**
```bash
g++ -std=c++20 -O3 -march=native simple_example.cpp -o simple_example
./simple_example
```

### **hybrid_api_example.cpp**
Comprehensive demonstration of the hybrid API design:
- Hot path performance patterns
- Error handling scenarios
- Mixed usage patterns
- Real-world configuration examples

**Compile & Run:**
```bash
g++ -std=c++20 -O3 -march=native hybrid_api_example.cpp -o hybrid_example
./hybrid_example
```

### **comprehensive_test.cpp**
Full test suite including:
- Functional validation tests
- Edge case testing
- Performance comparison
- Static optimization validation
- Memory efficiency tests

**Compile & Run:**
```bash
g++ -std=c++20 -O3 -march=native comprehensive_test.cpp -o comprehensive_test
./comprehensive_test
```

### **performance_benchmark.cpp**
Detailed performance benchmarking:
- Hybrid API performance comparison
- Exception vs noexcept overhead analysis
- Multiple iteration averaging
- Detailed performance metrics

**Compile & Run:**
```bash
g++ -std=c++20 -O3 -march=native performance_benchmark.cpp -o benchmark
./benchmark
```

## 🚀 Quick Start

For first-time users, start with `simple_example.cpp`:

```cpp
#include "../lfu_cache.h"

// Create cache: <Key, Value, MaxSize> - MaxSize is the capacity
LFUCache<int, std::string, 1000> cache; // Capacity is 1000 (compile-time)

// Hot path - maximum performance
cache.put(1, "value");
auto result = cache.get(1);  // noexcept, returns "" if not found

// Error handling when needed
try {
    auto value = cache.getOrThrow(999);  // Throws if not found
} catch (const std::runtime_error& e) {
    std::cout << "Key not found: " << e.what() << std::endl;
}
```

## 📊 Expected Performance

When compiled with `-O3 -march=native`, you should see:
- **20M+ operations/second** on modern hardware
- **Sub-microsecond** average operation time
- **Consistent O(1)** performance regardless of cache size

## 🔧 Compilation Tips

### **Maximum Performance:**
```bash
g++ -std=c++20 -O3 -march=native -DNDEBUG -flto example.cpp
```

### **Debug Build:**
```bash
g++ -std=c++20 -O0 -g -fsanitize=address example.cpp
```

### **With CMake:**
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
./lfu_example
```

## 🎯 Use Case Examples

### **🎮 Game Development - Optimal Scenarios:**

**Asset Caching (Known Max Assets):**
```cpp
// Texture cache - max 2000 textures known at compile-time
LFUCache<TextureId, Texture, 2000> textureCache;

// Critical rendering loop - predictable max, unpredictable access
for (auto& renderable : visibleObjects) {
    if (textureCache.contains(renderable.textureId)) {
        auto texture = textureCache.get(renderable.textureId);  // noexcept
        render(renderable, texture);
    }
}
```

**Event System (Infrequent but Unpredictable):**
```cpp
// Achievement cache - max 500 achievements, accessed unpredictably
LFUCache<AchievementId, Achievement, 500> achievementCache;

// Player unlocks achievement (rare event, frequency-based eviction ideal)
void onPlayerAction(PlayerId player, ActionType action) {
    auto achievement = achievementCache.get(getAchievementForAction(action));
    if (!achievement.name.empty()) {  // Found in cache
        showAchievementPopup(player, achievement);
    }
}
```

**Audio Sample Cache:**
```cpp
// Sound effects - max 1000 samples, LFU perfect for audio
LFUCache<SoundId, AudioSample, 1000> audioCache;

// Common sounds stay cached, rare sounds get evicted
void playSound(SoundId id) {
    auto sample = audioCache.get(id);
    if (!sample.data.empty()) {
        audioEngine.play(sample);  // Fast path for frequent sounds
    } else {
        loadAndCacheSound(id);     // Rare sounds loaded on demand
    }
}
```

### **🌐 Enterprise Applications:**

**Web Server Cache:**
```cpp
// Session cache for web server
LFUCache<std::string, UserSession, 10000> sessionCache;

// Hot path - validate session
if (sessionCache.contains(sessionId)) {
    auto session = sessionCache.get(sessionId);  // noexcept
    return session;
}
```

**Database Buffer Pool:**
```cpp
// Page cache for database
LFUCache<PageId, Page, 100000> bufferPool;

// Critical path - no exceptions
auto page = bufferPool.getOrDefault(pageId, Page::empty());
```

---

**Need help?** Check the main [README.md](../README.md) for comprehensive documentation.