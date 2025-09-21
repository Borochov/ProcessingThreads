cmake_minimum_required(VERSION 3.16)
project(ProcessingThreads VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4)
endif()

# Include directories
include_directories(include)

# Find required packages
find_package(Threads REQUIRED)

# Create the thread library
add_library(thread_lib STATIC
    src/threads.cpp
)

target_include_directories(thread_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(thread_lib PUBLIC
    Threads::Threads
)

target_compile_features(thread_lib PUBLIC cxx_std_17)

# Create main executable (processing_threads)
add_executable(processing_threads
    src/main.cpp
)

target_link_libraries(processing_threads
    thread_lib
)

# Create test executable
add_executable(test_runner
    tests/test_main.cpp
)

target_link_libraries(test_runner
    thread_lib
)

target_compile_features(test_runner PRIVATE cxx_std_17)

# Enable testing
enable_testing()
add_test(NAME basic_test COMMAND test_runner)

# Debug/Release configurations
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# Installation
install(TARGETS processing_threads DESTINATION bin)
install(TARGETS test_runner DESTINATION bin)
