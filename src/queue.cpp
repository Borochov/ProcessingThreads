#include "queue.h"

#include <iostream>

using namespace std;

// Constructor
template <typename T>
Queue<T>::Queue() : uniqueId(runningID++) {
    cout << "Created Queue of type: " << typeid(T).name() << ", uniqueId: " << uniqueId
         << ", Max Capacity: " << MAX_CAPACITY << endl;
}

template <typename T>
void Queue<T>::push(const T& elem) {
    std::unique_lock<std::mutex> lock(mtx);

    // Wait until queue has space
    cv.wait(lock, [this] { return elements.size() < MAX_CAPACITY; });

    elements.push(elem);
    cv.notify_one();  // Notify waiting pop operations
}

template <typename T>
T Queue<T>::pop() {
    std::unique_lock<std::mutex> lock(mtx);

    // Wait until the queue has elements
    cv.wait(lock, [this] { return !elements.empty(); });

    T elem = elements.front();
    elements.pop();
    cv.notify_one();  // Notify waiting push operations
    return elem;
}

template <typename T>
size_t Queue<T>::size() const {
    std::lock_guard<std::mutex> lock(mtx);
    return elements.size();
}

template <typename T>
bool Queue<T>::empty() const {
    std::lock_guard<std::mutex> lock(mtx);
    return elements.empty();
}

template <typename T>
int Queue<T>::getId() const {
    return uniqueId;
}

// Explicitly instantiate for the types you need
template class Queue<int>;
template class Queue<std::string>;
template class Queue<double>;
