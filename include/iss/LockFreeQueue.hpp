#include <iostream>
#include <atomic>
#include <vector>
#include <queue>
#include <cassert>

template <typename T>
class LockQueue {
public:
    // Constructor
    LockQueue() = default;

    // Destructor
    ~LockQueue() = default;

    // Enqueue an element
    void Enqueue(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        condition_.notify_one();
    }

    // Enqueue a batch of elements
    void EnqueueBatch(const std::vector<T>& values) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const T& value : values) {
            queue_.push(value);
        }
        condition_.notify_all();
    }

    // Dequeue an element
    T Dequeue() {
        std::unique_lock<std::mutex> lock(mutex_);
        // Wait until the queue is not empty
        condition_.wait(lock, [this] { return !queue_.empty(); });

        T value = queue_.front();
        queue_.pop();
        return value;
    }

    // Dequeue a batch of elements
    void DequeueBatch(std::vector<T>& values, std::size_t batchSize) {
        std::unique_lock<std::mutex> lock(mutex_);
        // Wait until the queue has enough elements
        condition_.wait(lock, [this, batchSize] { return queue_.size() >= batchSize; });

        for (std::size_t i = 0; i < batchSize; ++i) {
            values.push_back(queue_.front());
            queue_.pop();
            assert(i == values.back());
        }
    }

    // Check if the queue is empty
    bool IsEmpty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
};


template <typename T>
class LockFreeQueue {
public:
    // Constructor
    LockFreeQueue() : head_(nullptr), tail_(nullptr) {}

    // Destructor
    ~LockFreeQueue() {
        while (Node* oldHead = head_) {
            head_ = oldHead->next;
            delete oldHead;
        }
    }

    // Enqueue a batch of elements
    void EnqueueBatch(const std::vector<T>& values) {
        for (const T& value : values) {
            Enqueue(value);
        }
    }

    // Enqueue an element
    void Enqueue(const T& value) {
        Node* newNode = new Node(value);
        Node* oldTail = tail_.exchange(newNode, std::memory_order_acq_rel);
        if (oldTail) {
            oldTail->next = newNode;
        } else {
            head_ = newNode;
        }
    }

    // Dequeue an element
    bool Dequeue(T& value) {
        Node* oldHead = head_.load(std::memory_order_relaxed);
        if (!oldHead) {
            return false; // Queue is empty
        }

        head_ = oldHead->next;
        value = oldHead->data;
        delete oldHead;
        return true;
    }

    // Dequeue a batch of elements
    void DequeueBatch(std::vector<T>& values, std::size_t batchSize) {
        Node* oldHead = head_.load(std::memory_order_relaxed);
        std::size_t count = 0;
        while (count < batchSize) {
            if(oldHead) continue;
            Node* next = oldHead->next;
            values.push_back(oldHead->data);
            delete oldHead;
            oldHead = next;
            ++count;
        }

        head_ = oldHead;
    }

    // Check if the queue is empty
    bool IsEmpty() const {
        return head_.load(std::memory_order_relaxed) == nullptr;
    }

private:
    struct Node {
        T data;
        Node* next;

        explicit Node(const T& value) : data(value), next(nullptr) {}
    };

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
};