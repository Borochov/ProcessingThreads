cmake_minimum_required(VERSION 3.16)
project(MyProject VERSION 1.0.0 LANGUAGES CXX)

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

# Create a library for thread classes
add_library(thread_lib src/threads.cpp)
target_include_directories(thread_lib PUBLIC include)
target_compile_features(thread_lib PUBLIC cxx_std_17)

# Link threads library
find_package(Threads REQUIRED)
target_link_libraries(thread_lib PUBLIC Threads::Threads)

# Add main executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
)

# Link the thread library to main executable
target_link_libraries(${PROJECT_NAME} thread_lib)

# Enable testing
enable_testing()
add_subdirectory(tests)

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
