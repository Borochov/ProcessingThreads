#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "queue.h"
#include "threads.h"

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

// Original comprehensive queue tests
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

// Test DataValue variant
void test_data_value_variant() {
    std::cout << "\n=== Testing DataValue Variant ===" << std::endl;

    Queue<DataValue> queue;

    // Test with int
    DataValue int_val = 42;
    queue.push(int_val);
    TEST(queue.size() == 1, "Queue should have 1 element after pushing int");

    // Test with float
    DataValue float_val = 3.14f;
    queue.push(float_val);
    TEST(queue.size() == 2, "Queue should have 2 elements after pushing float");

    // Test with complex
    DataValue complex_val = std::complex<double>(1.0, 2.0);
    queue.push(complex_val);
    TEST(queue.size() == 3, "Queue should have 3 elements after pushing complex");

    // Pop and verify order (FIFO)
    auto val1 = queue.pop();
    TEST(std::holds_alternative<int>(val1), "First value should be int");
    TEST(std::get<int>(val1) == 42, "First value should be 42");

    auto val2 = queue.pop();
    TEST(std::holds_alternative<float>(val2), "Second value should be float");
    TEST(std::get<float>(val2) == 3.14f, "Second value should be 3.14");

    auto val3 = queue.pop();
    TEST(std::holds_alternative<std::complex<double>>(val3), "Third value should be complex");
    auto complex_result = std::get<std::complex<double>>(val3);
    TEST(complex_result.real() == 1.0 && complex_result.imag() == 2.0,
         "Complex value should be (1,2)");

    TEST(queue.empty(), "Queue should be empty after popping all values");
}

// Test single data thread
void test_single_data_thread() {
    std::cout << "\n=== Testing Single Data Thread ===" << std::endl;

    DataThread thread(1);

    // Let it run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    TEST(thread.get_id() == 1, "Thread ID should be 1");
    TEST(thread.get_queue_id() >= 0, "Queue ID should be valid");
    TEST(thread.is_running(), "Thread should be running");

    // Check if it generated some data
    size_t initial_size = thread.get_queue_size();
    TEST(initial_size > 0, "Thread should have generated some data");

    // Consume a value and verify it's one of our expected types
    if (!thread.is_queue_empty()) {
        auto value = thread.pop_value();
        bool is_valid_type = std::holds_alternative<int>(value) ||
                             std::holds_alternative<float>(value) ||
                             std::holds_alternative<std::complex<double>>(value);
        TEST(is_valid_type, "Generated value should be int, float, or complex");
    }

    thread.stop();

    // Give thread time to stop
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// Test multiple data threads
void test_multiple_data_threads() {
    std::cout << "\n=== Testing Multiple Data Threads ===" << std::endl;

    const int num_threads = 3;
    std::vector<std::unique_ptr<DataThread>> threads;

    // Create threads
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::make_unique<DataThread>(i + 1));
    }

    TEST(threads.size() == num_threads, "Should have created correct number of threads");

    // Verify all threads have unique queue IDs
    std::vector<int> queue_ids;
    for (const auto& thread : threads) {
        queue_ids.push_back(thread->get_queue_id());
    }

    // Check uniqueness
    bool unique_ids = true;
    for (size_t i = 0; i < queue_ids.size(); ++i) {
        for (size_t j = i + 1; j < queue_ids.size(); ++j) {
            if (queue_ids[i] == queue_ids[j]) {
                unique_ids = false;
                break;
            }
        }
    }
    TEST(unique_ids, "All threads should have unique queue IDs");

    // Let threads run
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    // Verify all threads generated data
    bool all_generated_data = true;
    for (const auto& thread : threads) {
        if (thread->get_queue_size() == 0) {
            all_generated_data = false;
            break;
        }
    }
    TEST(all_generated_data, "All threads should have generated some data");

    // Stop all threads
    for (auto& thread : threads) {
        thread->stop();
    }

    std::cout << "Thread queue sizes: ";
    for (const auto& thread : threads) {
        std::cout << "[" << thread->get_id() << ":" << thread->get_queue_size() << "] ";
    }
    std::cout << std::endl;
}

// Test data thread with specified number (for command line testing)
void test_data_threads_with_count(int num_threads) {
    std::cout << "\n=== Testing " << num_threads << " Data Threads ===" << std::endl;

    std::vector<std::unique_ptr<DataThread>> threads;

    // Create and start threads
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::make_unique<DataThread>(i + 1));
        std::cout << "Created data thread " << (i + 1)
                  << " with queue ID: " << threads[i]->get_queue_id() << std::endl;
    }

    // Let them run for 2 seconds
    std::cout << "Letting threads run for 2 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Show statistics
    std::cout << "\nThread Statistics:" << std::endl;
    size_t total_items = 0;
    for (const auto& thread : threads) {
        size_t queue_size = thread->get_queue_size();
        total_items += queue_size;
        std::cout << "Thread " << thread->get_id() << " (Queue " << thread->get_queue_id()
                  << "): " << queue_size << " items" << std::endl;
    }

    std::cout << "Total items generated: " << total_items << std::endl;

    // Sample some values from each thread
    std::cout << "\nSample values from each thread:" << std::endl;
    for (auto& thread : threads) {
        if (!thread->is_queue_empty()) {
            auto value = thread->pop_value();
            std::cout << "Thread " << thread->get_id() << " sample: ";
            std::visit([](const auto& v) { std::cout << v; }, value);
            std::cout << std::endl;
        }
    }

    // Stop all threads
    std::cout << "\nStopping all threads..." << std::endl;
    for (auto& thread : threads) {
        thread->stop();
    }

    threads.clear();
    std::cout << "All threads stopped." << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "Running comprehensive tests..." << std::endl;

    try {
        // Run original queue tests
        test_constructor();
        test_single_push_pop();
        test_fifo_order();
        test_capacity_operations();
        test_unique_ids();
        test_different_types();
        test_multiple_operations();
        test_data_value_variant();

        // Run thread tests
        test_single_data_thread();
        test_multiple_data_threads();

        // If command line argument provided, test with that many threads
        if (argc > 1) {
            int num_threads = std::stoi(argv[1]);
            if (num_threads > 0) {
                test_data_threads_with_count(num_threads);
            }
        }

        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Tests passed: " << tests_passed << std::endl;
        std::cout << "Tests failed: " << tests_failed << std::endl;

        if (tests_failed == 0) {
            std::cout << "All tests passed!" << std::endl;
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