#ifndef THREADS_H
#define THREADS_H

#include <atomic>
#include <complex>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <variant>
#include <vector>

#include "queue.h"

// Data types that threads can generate
using DataValue = std::variant<int, float, std::complex<double>>;

// Base thread class
class BaseThread {
   protected:
    int thread_id;
    std::thread worker_thread;
    std::atomic<bool> should_stop{false};

    // Random number generation
    std::random_device rd;
    std::mt19937 gen;

   public:
    BaseThread(int id) : thread_id(id), gen(rd()) {}

    virtual ~BaseThread() {
        stop();
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }

    void start() { worker_thread = std::thread(&BaseThread::work_loop, this); }

    void stop() { should_stop = true; }

    int get_id() const { return thread_id; }

    bool is_running() const { return !should_stop && worker_thread.joinable(); }

   protected:
    virtual void work_loop() = 0;

    void log(const std::string& message) {
        std::cout << "[Thread " << thread_id << "] " << message << std::endl;
    }
};

// Data generation thread
class DataThread : public BaseThread {
   private:
    std::unique_ptr<Queue<DataValue>> data_queue;

    // Random generators for different data types
    std::uniform_int_distribution<> type_selector;
    std::uniform_int_distribution<> int_generator;
    std::uniform_real_distribution<float> float_generator;
    std::uniform_real_distribution<double> complex_generator;

   public:
    DataThread(int id)
        : BaseThread(id),
          data_queue(std::make_unique<Queue<DataValue>>()),
          type_selector(0, 2),
          int_generator(-1000, 1000),
          float_generator(-100.0f, 100.0f),
          complex_generator(-50.0, 50.0) {
        log("Data thread created with queue ID: " + std::to_string(data_queue->getId()));
        start();
    }

    int get_queue_id() const { return data_queue->getId(); }

    size_t get_queue_size() const { return data_queue->size(); }

    bool is_queue_empty() const { return data_queue->empty(); }

    // For testing - consume a value from the queue
    DataValue pop_value() { return data_queue->pop(); }

   protected:
    void work_loop() override {
        log("Started working");

        while (!should_stop) {
            try {
                DataValue value = generate_random_value();
                data_queue->push(value);

                log_generated_value(value);

                // Variable sleep time based on thread ID to create different patterns
                std::this_thread::sleep_for(std::chrono::milliseconds(200 + (thread_id % 5) * 50));

            } catch (const std::exception& e) {
                log("Error: " + std::string(e.what()));
                break;
            }
        }

        log("Finished working");
    }

   private:
    DataValue generate_random_value() {
        int data_type = type_selector(gen);

        switch (data_type) {
            case 0:  // int
                return int_generator(gen);
            case 1:  // float
                return float_generator(gen);
            case 2:  // complex
                return std::complex<double>(complex_generator(gen), complex_generator(gen));
            default:
                return 0;  // fallback
        }
    }

    void log_generated_value(const DataValue& value) {
        std::string message = "Generated: ";

        // Use std::visit with proper handling for each type
        std::visit(
            [&message](const auto& v) {
                if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::complex<double>>) {
                    // Handle complex numbers specially
                    message +=
                        "(" + std::to_string(v.real()) + "," + std::to_string(v.imag()) + ")";
                } else {
                    // Handle int and float
                    message += std::to_string(v);
                }
            },
            value);

        message += " (queue size: " + std::to_string(data_queue->size()) + ")";
        log(message);
    }
};

#endif  // THREADS_H