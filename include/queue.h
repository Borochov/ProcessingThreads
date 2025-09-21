#ifndef QUEUE_H
#define QUEUE_H

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

using namespace std;

// Global counter shared by all Queue instantiations
static atomic<int> globalRunningID{0};

template <typename T>
class Queue {
   public:
    // Constructor with dynamic capacity
    Queue(int capacity = 50) : maxCapacity(capacity), uniqueId(globalRunningID++) {
        cout << "Created Queue of type: " << typeid(T).name() << ", uniqueId: " << uniqueId
             << ", Max Capacity: " << maxCapacity << endl;
    }

    void push(const T& elem) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this] { return elements.size() < static_cast<size_t>(maxCapacity); });
        elements.push(elem);
        cv.notify_one();
    }

    T pop() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this] { return !elements.empty(); });
        T elem = elements.front();
        elements.pop();
        cv.notify_one();
        return elem;
    }

    size_t size() const {
        lock_guard<mutex> lock(mtx);
        return elements.size();
    }

    bool empty() const {
        lock_guard<mutex> lock(mtx);
        return elements.empty();
    }

    int getId() const { return uniqueId; }

    int getMaxCapacity() const { return maxCapacity; }

   private:
    queue<T> elements;
    int uniqueId;
    int maxCapacity;
    mutable mutex mtx;
    condition_variable cv;
};

#endif
