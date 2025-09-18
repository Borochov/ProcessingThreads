#ifndef QUEUE_H
#define QUEUE_H

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

// Global counter shared by all Queue instantiations
static std::atomic<int> globalRunningID{0};

template <typename T>
class Queue {
   public:
    // Constructor
    Queue() : uniqueId(globalRunningID++) {
        std::cout << "Created Queue of type: " << typeid(T).name() << ", uniqueId: " << uniqueId
                  << ", Max Capacity: " << MAX_CAPACITY << std::endl;
    }

    void push(const T& elem) {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait until queue has space
        cv.wait(lock, [this] { return elements.size() < MAX_CAPACITY; });

        elements.push(elem);
        cv.notify_one();  // Notify waiting pop operations
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait until the queue has elements
        cv.wait(lock, [this] { return !elements.empty(); });

        T elem = elements.front();
        elements.pop();
        cv.notify_one();  // Notify waiting push operations
        return elem;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return elements.size();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return elements.empty();
    }

    int getId() const { return uniqueId; }

    static const int MAX_CAPACITY = 10;

   private:
    std::queue<T> elements;
    int uniqueId;
    mutable std::mutex mtx;
    std::condition_variable cv;
};

#endif
