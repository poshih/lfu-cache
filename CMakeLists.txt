cmake_minimum_required(VERSION 3.10)
project(LFUCache
    VERSION 1.0.0
    DESCRIPTION "High-Performance LFU Cache with Hybrid API"
    LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG -flto")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fsanitize=address -fsanitize=undefined")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /GL")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1")
endif()

# Header-only library
add_library(lfu_cache INTERFACE)
target_include_directories(lfu_cache INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

# Example executable
add_executable(lfu_example hybrid_api_example.cpp)
target_link_libraries(lfu_example lfu_cache)

# Test executable
add_executable(lfu_test test_optimized_lfu.cpp)
target_link_libraries(lfu_test lfu_cache)

# Benchmark executable
add_executable(lfu_benchmark hybrid_benchmark.cpp)
target_link_libraries(lfu_benchmark lfu_cache)

# Enable testing
enable_testing()
add_test(NAME lfu_functionality_test COMMAND lfu_test)

# Installation
include(GNUInstallDirs)

install(FILES lfu_cache.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS lfu_cache)
    EXPORT lfu_cache-targets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT lfu_cache-targets
    FILE lfu_cache-targets.cmake
    NAMESPACE LFUCache::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lfu_cache)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lfu_cache-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/lfu_cache-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lfu_cache-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lfu_cache)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/lfu_cache-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/lfu_cache-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lfu_cache)