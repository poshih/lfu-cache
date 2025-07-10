# Contributing to High-Performance LFU Cache

Thank you for your interest in contributing to the High-Performance LFU Cache project! This document provides guidelines and information for contributors.

## 🤝 Code of Conduct

This project adheres to a code of conduct. By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

## 🐛 Reporting Issues

Before creating an issue, please:

1. **Search existing issues** to avoid duplicates
2. **Use the latest version** to ensure the issue still exists
3. **Provide detailed information** including:
   - Operating system and version
   - Compiler and version (GCC, Clang, MSVC)
   - Compilation flags used
   - Minimal code example reproducing the issue
   - Expected vs. actual behavior

### Issue Template

```markdown
**Environment:**
- OS: [e.g., Ubuntu 20.04, Windows 10, macOS 12]
- Compiler: [e.g., GCC 11.2, Clang 13.0, MSVC 2019]
- Compilation flags: [e.g., -std=c++17 -O3 -march=native]

**Description:**
Brief description of the issue

**Minimal Example:**
```cpp
// Minimal code that reproduces the issue
```

**Expected Behavior:**
What you expected to happen

**Actual Behavior:**
What actually happened

**Additional Context:**
Any other relevant information
```

## 🚀 Contributing Code

### Getting Started

1. **Fork** the repository
2. **Clone** your fork: `git clone https://github.com/yourusername/lfu-cache.git`
3. **Create a branch**: `git checkout -b feature/your-feature-name`
4. **Make your changes**
5. **Test thoroughly**
6. **Submit a pull request**

### Development Setup

```bash
# Clone the repository
git clone https://github.com/poshihtsang/lfu-cache.git
cd lfu-cache

# Build with CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Run tests
ctest --output-on-failure

# Run benchmarks
./lfu_benchmark
```

### Code Style Guidelines

1. **C++17 Standard**: Use modern C++ features appropriately
2. **Performance First**: Maintain O(1) complexity guarantees
3. **Documentation**: Document all public APIs
4. **Testing**: Include tests for new features
5. **Benchmarking**: Include performance measurements for optimizations

#### Formatting Rules

- **Indentation**: 4 spaces (no tabs)
- **Line Length**: 100 characters maximum
- **Naming**:
  - Classes: `PascalCase` (e.g., `LFUCache`)
  - Functions/Variables: `camelCase` (e.g., `updateFrequency`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MIN_FREQUENCY_SIZE`)
  - Private members: `trailing_` (e.g., `capacity_`)

#### Example Code Style

```cpp
template<typename Key, typename Value, size_t MaxSize>
class ExampleClass {
private:
    static constexpr size_t DEFAULT_SIZE = 16;
    int memberVariable_;
    
public:
    explicit ExampleClass(int capacity) : memberVariable_(capacity) {
        // Constructor implementation
    }
    
    inline bool checkCondition() const noexcept {
        return memberVariable_ > 0;
    }
};
```

### Performance Requirements

All contributions must maintain performance characteristics:

- **O(1) complexity** for get() and put() operations
- **No heap allocation** during normal operation
- **Cache-friendly** memory access patterns
- **Exception safety** without performance penalty

### Testing Requirements

1. **Functionality Tests**: All new features must include tests
2. **Performance Tests**: Optimizations must include benchmarks
3. **Regression Tests**: Ensure no performance degradation
4. **Edge Cases**: Test boundary conditions and error cases

#### Running Tests

```bash
# Functionality tests
./lfu_test

# Performance benchmarks
./lfu_benchmark

# Memory tests (requires Valgrind)
valgrind --tool=memcheck --leak-check=full ./lfu_test

# Static analysis (requires clang-tidy)
clang-tidy lfu_cache.h -- -std=c++20
```

## 📊 Performance Benchmarking

When submitting performance improvements:

1. **Include benchmarks** comparing before/after performance
2. **Use consistent hardware** for measurements
3. **Report compiler and flags** used for benchmarking
4. **Include multiple test scenarios** (hot/cold cache, different sizes)

### Benchmark Template

```cpp
// Before optimization: X.XX M ops/sec
// After optimization:  Y.YY M ops/sec  
// Improvement: Z.Z% faster

// Test environment:
// - Hardware: Intel i7-9750H @ 2.60GHz
// - Compiler: GCC 11.2 -O3 -march=native
// - Test: 1M operations, cache size 1000
```

## 🔍 Static Analysis

We use static analysis tools to maintain code quality:

- **Clang-Tidy**: For code quality and best practices
- **AddressSanitizer**: For memory safety
- **UBSan**: For undefined behavior detection
- **Valgrind**: For memory leak detection

## 📝 Documentation

When adding features:

1. **Update README.md** with new functionality
2. **Add code examples** in header comments
3. **Update CHANGELOG.md** with your changes
4. **Include performance impact** in documentation

## 🏷️ Pull Request Guidelines

### PR Checklist

- [ ] **Tests pass**: All existing tests continue to pass
- [ ] **New tests added**: New functionality includes tests
- [ ] **Performance verified**: No regression in performance
- [ ] **Documentation updated**: README and code comments updated
- [ ] **Code style followed**: Consistent with existing codebase
- [ ] **No breaking changes**: Unless clearly documented

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Performance improvement
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update

## Performance Impact
- Benchmark results showing performance impact
- Any memory usage changes

## Testing
- Description of tests added
- How the change was tested

## Checklist
- [ ] Tests pass locally
- [ ] Code follows project style guidelines
- [ ] Documentation updated
- [ ] Performance verified
```

## 🎯 Areas for Contribution

We welcome contributions in these areas:

### High Priority
- **Thread-safe variant** with lock-free operations
- **NUMA-aware allocators** for large-scale deployments
- **Template specializations** for common types
- **C++20 concepts** integration

### Medium Priority
- **Serialization support** for persistence
- **Statistics interfaces** for monitoring
- **Custom allocator support**
- **Constexpr improvements** where possible

### Low Priority
- **Additional hash functions** for specific use cases
- **Debug visualization tools**
- **Additional benchmarking scenarios**

## 🏆 Recognition

Contributors will be:
- **Listed in CONTRIBUTORS.md**
- **Credited in release notes**
- **Acknowledged in relevant documentation**

## 📬 Contact

For questions about contributing:
- **Open an issue** for public discussion
- **Email maintainer**: [Contact via GitHub profile]
- **Discussion forum**: GitHub Discussions

---

Thank you for contributing to making this the fastest LFU cache implementation! 🚀