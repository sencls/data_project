#pragma once
#include <stdexcept>

template<typename T>
class Queue {
public:
    Queue() : buffer_(new T[16]), capacity_(16), front_(0), back_(0), size_(0) {}

    explicit Queue(int cap) : buffer_(new T[cap]), capacity_(cap), front_(0), back_(0), size_(0) {}

    ~Queue() { delete[] buffer_; }

    Queue(const Queue& other)
        : buffer_(new T[other.capacity_]), capacity_(other.capacity_),
          front_(0), back_(other.size_), size_(other.size_) {
        for (int i = 0; i < other.size_; i++) {
            buffer_[i] = other.buffer_[(other.front_ + i) % other.capacity_];
        }
    }

    Queue(Queue&& other) noexcept
        : buffer_(other.buffer_), capacity_(other.capacity_),
          front_(other.front_), back_(other.back_), size_(other.size_) {
        other.buffer_ = nullptr;
        other.capacity_ = 0;
        other.front_ = 0;
        other.back_ = 0;
        other.size_ = 0;
    }

    Queue& operator=(const Queue& other) {
        if (this != &other) {
            delete[] buffer_;
            capacity_ = other.capacity_;
            buffer_ = new T[capacity_];
            front_ = 0;
            size_ = other.size_;
            back_ = size_;
            for (int i = 0; i < size_; i++) {
                buffer_[i] = other.buffer_[(other.front_ + i) % other.capacity_];
            }
        }
        return *this;
    }

    void enqueue(const T& val) {
        ensureCapacity();
        buffer_[back_] = val;
        back_ = (back_ + 1) % capacity_;
        size_++;
    }

    T dequeue() {
        if (size_ == 0) throw std::runtime_error("Queue is empty");
        T val = buffer_[front_];
        front_ = (front_ + 1) % capacity_;
        size_--;
        return val;
    }

    T& peek() {
        if (size_ == 0) throw std::runtime_error("Queue is empty");
        return buffer_[front_];
    }

    const T& peek() const {
        if (size_ == 0) throw std::runtime_error("Queue is empty");
        return buffer_[front_];
    }

    int size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void clear() {
        front_ = 0;
        back_ = 0;
        size_ = 0;
    }

private:
    T* buffer_;
    int capacity_;
    int front_;
    int back_;
    int size_;

    void ensureCapacity() {
        if (size_ >= capacity_ - 1) {
            int newCap = capacity_ * 2;
            T* newBuf = new T[newCap];
            for (int i = 0; i < size_; i++) {
                newBuf[i] = buffer_[(front_ + i) % capacity_];
            }
            delete[] buffer_;
            buffer_ = newBuf;
            capacity_ = newCap;
            front_ = 0;
            back_ = size_;
        }
    }
};
