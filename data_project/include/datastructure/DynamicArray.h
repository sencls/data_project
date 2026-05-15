#pragma once
#include <stdexcept>
#include <initializer_list>

template<typename T>
class DynamicArray {
public:
    DynamicArray() : data_(nullptr), size_(0), capacity_(0) {}

    explicit DynamicArray(int cap) : size_(0), capacity_(cap) {
        data_ = new T[capacity_];
    }

    DynamicArray(std::initializer_list<T> init) : size_(0), capacity_(init.size()) {
        data_ = new T[capacity_];
        for (const auto& v : init) push_back(v);
    }

    DynamicArray(const DynamicArray& other) : size_(other.size_), capacity_(other.capacity_) {
        data_ = new T[capacity_];
        for (int i = 0; i < size_; i++) data_[i] = other.data_[i];
    }

    DynamicArray(DynamicArray&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    ~DynamicArray() { delete[] data_; }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = new T[capacity_];
            for (int i = 0; i < size_; i++) data_[i] = other.data_[i];
        }
        return *this;
    }

    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    void push_back(const T& val) {
        ensureCapacity(size_ + 1);
        data_[size_++] = val;
    }

    void pop_back() {
        if (size_ > 0) size_--;
    }

    T& operator[](int index) { return data_[index]; }
    const T& operator[](int index) const { return data_[index]; }

    T& at(int index) {
        if (index < 0 || index >= size_) throw std::out_of_range("DynamicArray index out of range");
        return data_[index];
    }

    int size() const { return size_; }
    int capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

    T& front() { return data_[0]; }
    T& back() { return data_[size_ - 1]; }
    const T& front() const { return data_[0]; }
    const T& back() const { return data_[size_ - 1]; }

    void clear() { size_ = 0; }

    void reserve(int cap) {
        if (cap > capacity_) {
            T* newData = new T[cap];
            for (int i = 0; i < size_; i++) newData[i] = data_[i];
            delete[] data_;
            data_ = newData;
            capacity_ = cap;
        }
    }

    int find(const T& val) const {
        for (int i = 0; i < size_; i++) {
            if (data_[i] == val) return i;
        }
        return -1;
    }

    bool removeAt(int index) {
        if (index < 0 || index >= size_) return false;
        for (int i = index; i < size_ - 1; i++) data_[i] = data_[i + 1];
        size_--;
        return true;
    }

private:
    T* data_;
    int size_;
    int capacity_;

    void ensureCapacity(int needed) {
        if (needed > capacity_) {
            int newCap = (capacity_ == 0) ? 4 : capacity_ * 2;
            while (newCap < needed) newCap *= 2;
            reserve(newCap);
        }
    }
};
