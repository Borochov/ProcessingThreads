#ifndef QUEUE_H
#define QUEUE_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class Queue {
   public:
    Queue();  // Constructor
    void push(const T& elem);
    T pop();
    size_t size() const;
    bool empty() const;
    int getId() const;

    static const int MAX_CAPACITY = 10;

   private:
    std::queue<T> elements;
    static std::atomic<int> runningID;
    int uniqueId;
    mutable std::mutex mtx;
    std::condition_variable cv;
};

template <typename T>
std::atomic<int> Queue<T>::runningID{0};

#endif
