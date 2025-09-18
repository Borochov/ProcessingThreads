#include <cassert>
#include <iostream>

#include "queue.h"

// Test result tracking
int tests_passed = 0;
int tests_failed = 0;

#define TEST(condition, message)                           \
    do {                                                   \
        if (condition) {                                   \
            std::cout << "PASS: " << message << std::endl; \
            tests_passed++;                                \
        } else {                                           \
            std::cout << "FAIL: " << message << std::endl; \
            tests_failed++;                                \
        }                                                  \
    } while (0)

void test_constructor() {
    std::cout << "\n=== Testing Constructor ===" << std::endl;

    Queue<int> queue;

    // Test initial state
    TEST(queue.empty(), "Queue should be empty initially");
    TEST(queue.size() == 0, "Queue size should be 0 initially");
}

void test_single_push_pop() {
    std::cout << "\n=== Testing Single Push/Pop ===" << std::endl;

    Queue<int> queue;

    // Test single push
    queue.push(42);
    TEST(!queue.empty(), "Queue should not be empty after push");
    TEST(queue.size() == 1, "Queue size should be 1 after push");

    // Test single pop
    int value = queue.pop();
    TEST(value == 42, "Popped value should match pushed value");
    TEST(queue.empty(), "Queue should be empty after pop");
    TEST(queue.size() == 0, "Queue size should be 0 after pop");
}

void test_fifo_order() {
    std::cout << "\n=== Testing FIFO Order ===" << std::endl;

    Queue<int> queue;

    // Push multiple elements
    for (int i = 1; i <= 5; ++i) {
        queue.push(i);
    }

    TEST(queue.size() == 5, "Queue should contain 5 elements");

    // Pop and verify FIFO order
    for (int i = 1; i <= 5; ++i) {
        int value = queue.pop();
        TEST(value == i, "Elements should be popped in FIFO order");
    }

    TEST(queue.empty(), "Queue should be empty after all pops");
}

void test_capacity_operations() {
    std::cout << "\n=== Testing Capacity Operations ===" << std::endl;

    Queue<int> queue;

    // Fill queue to capacity
    for (int i = 0; i < Queue<int>::MAX_CAPACITY; ++i) {
        queue.push(i);
    }

    TEST(queue.size() == Queue<int>::MAX_CAPACITY, "Queue should be at max capacity");
    TEST(!queue.empty(), "Full queue should not be empty");

    // Test that we can still pop from full queue
    int first_value = queue.pop();
    TEST(first_value == 0, "First popped value should be 0");
    TEST(queue.size() == Queue<int>::MAX_CAPACITY - 1, "Queue size should decrease after pop");

    // Verify we can push again after popping
    queue.push(999);
    TEST(queue.size() == Queue<int>::MAX_CAPACITY, "Queue should be full again after push");
}

void test_unique_ids() {
    std::cout << "\n=== Testing Unique IDs ===" << std::endl;

    Queue<int> queue1;
    Queue<int> queue2;
    Queue<std::string> queue3;

    int id1 = queue1.getId();
    int id2 = queue2.getId();
    int id3 = queue3.getId();

    TEST(id1 != id2, "Different queue instances should have different IDs");
    TEST(id2 != id3, "Different queue instances should have different IDs");
    TEST(id1 != id3, "Different queue instances should have different IDs");

    std::cout << "Queue IDs: " << id1 << ", " << id2 << ", " << id3 << std::endl;
}

void test_different_types() {
    std::cout << "\n=== Testing Different Types ===" << std::endl;

    // Test with strings
    Queue<std::string> string_queue;
    string_queue.push("hello");
    string_queue.push("world");

    std::string str1 = string_queue.pop();
    std::string str2 = string_queue.pop();

    TEST(str1 == "hello", "String queue should work correctly");
    TEST(str2 == "world", "String queue should maintain order");
    TEST(string_queue.empty(), "String queue should be empty after pops");

    // Test with doubles
    Queue<double> double_queue;
    double_queue.push(3.14);
    double_queue.push(2.71);

    double d1 = double_queue.pop();
    double d2 = double_queue.pop();

    TEST(d1 == 3.14, "Double queue should work correctly");
    TEST(d2 == 2.71, "Double queue should maintain order");
}

void test_multiple_operations() {
    std::cout << "\n=== Testing Multiple Operations ===" << std::endl;

    Queue<int> queue;

    // Mix push and pop operations
    queue.push(1);
    queue.push(2);

    int val1 = queue.pop();
    TEST(val1 == 1, "First pop should return 1");
    TEST(queue.size() == 1, "Queue size should be 1 after one pop");

    queue.push(3);
    queue.push(4);

    TEST(queue.size() == 3, "Queue size should be 3 after more pushes");

    int val2 = queue.pop();
    int val3 = queue.pop();
    int val4 = queue.pop();

    TEST(val2 == 2, "Second pop should return 2");
    TEST(val3 == 3, "Third pop should return 3");
    TEST(val4 == 4, "Fourth pop should return 4");
    TEST(queue.empty(), "Queue should be empty after all pops");
}

int main() {
    std::cout << "Running basic Queue tests..." << std::endl;

    try {
        test_constructor();
        test_single_push_pop();
        test_fifo_order();
        test_capacity_operations();
        test_unique_ids();
        test_different_types();
        test_multiple_operations();

        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Tests passed: " << tests_passed << std::endl;
        std::cout << "Tests failed: " << tests_failed << std::endl;

        if (tests_failed == 0) {
            std::cout << "All basic tests passed!" << std::endl;
            return 0;
        } else {
            std::cout << "Some tests failed!" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception during testing: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Unknown exception during testing!" << std::endl;
        return 1;
    }
}