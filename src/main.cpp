#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include "hello.h"

// Demonstrate C++17 features
auto process_data() {
    std::vector<std::string> data = {"hello", "world", "c++17", "cmake"};
    
    // C++17: structured bindings
    for (const auto& [index, value] : std::vector<std::pair<int, std::string>>{
        {1, "First"}, {2, "Second"}, {3, "Third"}
    }) {
        std::cout << "Index: " << index << ", Value: " << value << std::endl;
    }
    
    // C++17: if constexpr and fold expressions could be used here
    return data;
}

int main() {
    std::cout << "C++17 CMake Project" << std::endl;
    std::cout << "==================" << std::endl;
    
    // Use our header function
    print_hello("CMake Project");
    
    // C++17: auto type deduction
    auto data = process_data();
    
    std::cout << "\nData processed:" << std::endl;
    for (const auto& item : data) {
        std::cout << "- " << item << std::endl;
    }
    
    // C++17: optional could be demonstrated
    std::cout << "\nProject compiled successfully with C++17!" << std::endl;
    
    return 0;
}