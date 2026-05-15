#pragma once
#include "DynamicArray.h"
#include <functional>

template<typename T>
class PriorityQueue {
public:
    PriorityQueue() {}
    explicit PriorityQueue(std::function<bool(const T&, const T&)> cmp) : cmp_(cmp) {}

    void push(const T& val) {
        heap_.push_back(val);
        siftUp(heap_.size() - 1);
    }

    T pop() {
        T top = heap_.front();
        heap_.front() = heap_.back();
        heap_.pop_back();
        if (!heap_.empty()) siftDown(0);
        return top;
    }

    const T& top() const { return heap_.front(); }

    bool empty() const { return heap_.empty(); }
    int size() const { return heap_.size(); }

    void clear() { heap_.clear(); }

    void decreaseKey(int index, const T& newVal) {
        heap_[index] = newVal;
        siftUp(index);
    }

private:
    DynamicArray<T> heap_;
    std::function<bool(const T&, const T&)> cmp_ = [](const T& a, const T& b) { return a < b; };

    void siftUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (cmp_(heap_[i], heap_[parent])) {
                T tmp = heap_[i];
                heap_[i] = heap_[parent];
                heap_[parent] = tmp;
                i = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(int i) {
        int n = heap_.size();
        while (true) {
            int smallest = i;
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            if (left < n && cmp_(heap_[left], heap_[smallest])) smallest = left;
            if (right < n && cmp_(heap_[right], heap_[smallest])) smallest = right;
            if (smallest != i) {
                T tmp = heap_[i];
                heap_[i] = heap_[smallest];
                heap_[smallest] = tmp;
                i = smallest;
            } else {
                break;
            }
        }
    }
};
