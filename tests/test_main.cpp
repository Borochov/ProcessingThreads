#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "queue.h"
#include "threads.h"

using namespace std;

// Test result tracking
int tests_passed = 0;
int tests_failed = 0;

#define TEST(condition, message)                 \
    do {                                         \
        if (condition) {                         \
            cout << "PASS: " << message << endl; \
            tests_passed++;                      \
        } else {                                 \
            cout << "FAIL: " << message << endl; \
            tests_failed++;                      \
        }                                        \
    } while (0)

// Test basic queue functionality
void test_queue_basic_functionality() {
    cout << "\n=== Testing Queue Basic Functionality ===" << endl;

    Queue<int> queue;

    // Test FIFO behavior
    for (int i = 1; i <= 5; ++i) {
        queue.push(i);
    }

    for (int i = 1; i <= 5; ++i) {
        int value = queue.pop();
        TEST(value == i, "Queue maintains FIFO order");
    }

    TEST(queue.empty(), "Queue is empty after all pops");
}

// Test queue with different data types
void test_queue_with_variant() {
    cout << "\n=== Testing Queue with DataValue Types ===" << endl;

    Queue<DataValue> queue;

    // Push different types
    queue.push(42);
    queue.push(3.14f);
    queue.push(complex<double>(1.0, 2.0));

    // Verify we can pop them back in order
    auto val1 = queue.pop();
    auto val2 = queue.pop();
    auto val3 = queue.pop();

    TEST(holds_alternative<int>(val1) && get<int>(val1) == 42, "First value is correct int");
    TEST(holds_alternative<float>(val2) && get<float>(val2) == 3.14f,
         "Second value is correct float");
    TEST(holds_alternative<complex<double>>(val3), "Third value is complex number");
}

// Test data thread generation
void test_data_generation() {
    cout << "\n=== Testing Data Generation ===" << endl;

    DataThread thread(1);

    // Let it generate some data
    this_thread::sleep_for(chrono::milliseconds(600));

    TEST(thread.getQueueSize() > 0, "Data thread generates values");

    // Sample some values to verify type variety
    vector<DataValue> samples;
    for (int i = 0; i < 3 && !thread.isQueueEmpty(); ++i) {
        samples.push_back(thread.popValue());
    }

    TEST(!samples.empty(), "Can retrieve generated data");

    thread.stop();
}

// Test function generation
void test_function_generation() {
    cout << "\n=== Testing Function Generation ===" << endl;

    FunctionThread thread(1);

    // Let it generate some functions
    this_thread::sleep_for(chrono::milliseconds(800));

    TEST(thread.getQueueSize() > 0, "Function thread generates functions");

    // Sample a function and verify it's valid
    if (!thread.isQueueEmpty()) {
        auto func = thread.popFunction();

        bool valid_op = (func.op >= Operation::ADD && func.op <= Operation::DIVIDE);
        TEST(valid_op, "Generated function has valid operation");

        size_t args_needed = func.requiredArgs();
        TEST(args_needed <= 2, "Function requires reasonable number of arguments");

        cout << "Sample function: " << func.description() << endl;
    }

    thread.stop();
}

// Test multiple threads working together
void test_concurrent_operation() {
    cout << "\n=== Testing Concurrent Thread Operation ===" << endl;

    // Create multiple data and function threads
    vector<unique_ptr<DataThread>> data_threads;
    vector<unique_ptr<FunctionThread>> function_threads;

    for (int i = 0; i < 2; ++i) {
        data_threads.push_back(make_unique<DataThread>(i + 1));
        function_threads.push_back(make_unique<FunctionThread>(i + 10));
    }

    // Let them all work
    this_thread::sleep_for(chrono::milliseconds(1000));

    // Verify all threads produced output
    bool all_data_threads_working = true;
    bool all_function_threads_working = true;

    for (const auto& thread : data_threads) {
        if (thread->getQueueSize() == 0) {
            all_data_threads_working = false;
        }
    }

    for (const auto& thread : function_threads) {
        if (thread->getQueueSize() == 0) {
            all_function_threads_working = false;
        }
    }

    TEST(all_data_threads_working, "All data threads are producing data");
    TEST(all_function_threads_working, "All function threads are producing functions");

    // Verify threads have unique queue IDs
    vector<int> all_queue_ids;
    for (const auto& thread : data_threads) {
        all_queue_ids.push_back(thread->getQueueId());
    }
    for (const auto& thread : function_threads) {
        all_queue_ids.push_back(thread->getQueueId());
    }

    bool unique_ids = true;
    for (size_t i = 0; i < all_queue_ids.size(); ++i) {
        for (size_t j = i + 1; j < all_queue_ids.size(); ++j) {
            if (all_queue_ids[i] == all_queue_ids[j]) {
                unique_ids = false;
                break;
            }
        }
    }

    TEST(unique_ids, "All queues have unique IDs");

    // Stop all threads
    for (auto& thread : data_threads) {
        thread->stop();
    }
    for (auto& thread : function_threads) {
        thread->stop();
    }
}

// Test arithmetic function execution
void test_arithmetic_function_evaluation() {
    cout << "\n=== Testing Arithmetic Function Structure ===" << endl;

    // Test different function patterns
    ArithmeticFunction func1;
    func1.op = Operation::ADD;
    // x + y pattern
    TEST(func1.requiredArgs() == 2, "Binary function needs 2 args");

    ArithmeticFunction func2;
    func2.op = Operation::MULTIPLY;
    func2.right_operand = 5;
    // x * 5 pattern
    TEST(func2.requiredArgs() == 1, "Unary function needs 1 arg");

    ArithmeticFunction func3;
    func3.op = Operation::SUBTRACT;
    func3.left_operand = 10;
    func3.right_operand = 3;
    // 10 - 3 pattern
    TEST(func3.requiredArgs() == 0, "Constant function needs 0 args");

    cout << "Function examples:" << endl;
    cout << "  " << func1.description() << endl;
    cout << "  " << func2.description() << endl;
    cout << "  " << func3.description() << endl;
}

// Integration test with command line parameters
void test_with_parameters(int num_data_threads, int num_function_threads) {
    if (num_data_threads <= 0 || num_function_threads <= 0) {
        return;
    }

    cout << "\n=== Integration Test: " << num_data_threads << " Data + " << num_function_threads
         << " Function Threads ===" << endl;

    vector<unique_ptr<DataThread>> data_threads;
    vector<unique_ptr<FunctionThread>> function_threads;

    // Create threads
    for (int i = 0; i < num_data_threads; ++i) {
        data_threads.push_back(make_unique<DataThread>(i + 1));
    }

    for (int i = 0; i < num_function_threads; ++i) {
        function_threads.push_back(make_unique<FunctionThread>(i + 100));
    }

    cout << "Threads running for 2 seconds..." << endl;
    this_thread::sleep_for(chrono::seconds(2));

    // Show statistics
    size_t total_data = 0;
    size_t total_functions = 0;

    for (const auto& thread : data_threads) {
        total_data += thread->getQueueSize();
    }

    for (const auto& thread : function_threads) {
        total_functions += thread->getQueueSize();
    }

    cout << "Generated " << total_data << " data values and " << total_functions << " functions"
         << endl;

    // Stop all threads
    for (auto& thread : data_threads) {
        thread->stop();
    }
    for (auto& thread : function_threads) {
        thread->stop();
    }
}

int main(int argc, char* argv[]) {
    cout << "Running functional tests..." << endl;

    try {
        // Core functionality tests
        test_queue_basic_functionality();
        test_queue_with_variant();
        test_data_generation();
        test_function_generation();
        test_concurrent_operation();
        test_arithmetic_function_evaluation();

        // Integration test with command line parameters
        if (argc >= 3) {
            int num_data = stoi(argv[1]);
            int num_function = stoi(argv[2]);
            test_with_parameters(num_data, num_function);
        }

        cout << "\n=== Test Summary ===" << endl;
        cout << "Tests passed: " << tests_passed << endl;
        cout << "Tests failed: " << tests_failed << endl;

        if (tests_failed == 0) {
            cout << "All tests passed!" << endl;
            return 0;
        } else {
            cout << "Some tests failed!" << endl;
            return 1;
        }
    } catch (const exception& e) {
        cout << "Exception during testing: " << e.what() << endl;
        return 1;
    }
}
