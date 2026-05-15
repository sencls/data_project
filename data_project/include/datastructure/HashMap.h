#pragma once
#include "DynamicArray.h"
#include <functional>
#include <string>

template<typename K, typename V>
class HashMap {
    struct Entry {
        K key;
        V value;
        bool occupied = false;
        bool deleted = false;
    };

public:
    HashMap() : data_(nullptr), capacity_(0), count_(0) {
        growTo(16);
    }

    explicit HashMap(int initCap) : data_(nullptr), capacity_(0), count_(0) {
        growTo(nextPow2(initCap));
    }

    HashMap(const HashMap& other) : data_(nullptr), capacity_(0), count_(0) {
        growTo(other.capacity_);
        for (int i = 0; i < other.capacity_; i++) {
            if (other.data_[i].occupied) {
                insert(other.data_[i].key, other.data_[i].value);
            }
        }
    }

    HashMap(HashMap&& other) noexcept : data_(other.data_), capacity_(other.capacity_), count_(other.count_) {
        other.data_ = nullptr;
        other.capacity_ = 0;
        other.count_ = 0;
    }

    ~HashMap() { delete[] data_; }

    HashMap& operator=(const HashMap& other) {
        if (this != &other) {
            delete[] data_;
            data_ = nullptr;
            capacity_ = 0;
            count_ = 0;
            growTo(other.capacity_);
            for (int i = 0; i < other.capacity_; i++) {
                if (other.data_[i].occupied) {
                    insert(other.data_[i].key, other.data_[i].value);
                }
            }
        }
        return *this;
    }

    HashMap& operator=(HashMap&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            capacity_ = other.capacity_;
            count_ = other.count_;
            other.data_ = nullptr;
            other.capacity_ = 0;
            other.count_ = 0;
        }
        return *this;
    }

    void insert(const K& key, const V& val) {
        if (static_cast<double>(count_ + 1) / capacity_ > 0.75) {
            rehash(capacity_ * 2);
        }
        int idx = findIndex(key);
        if (!data_[idx].occupied) {
            data_[idx].key = key;
            data_[idx].value = val;
            data_[idx].occupied = true;
            data_[idx].deleted = false;
            count_++;
        } else {
            data_[idx].value = val;
        }
    }

    V& operator[](const K& key) {
        if (static_cast<double>(count_ + 1) / capacity_ > 0.75) {
            rehash(capacity_ * 2);
        }
        int idx = findIndex(key);
        if (!data_[idx].occupied) {
            data_[idx].key = key;
            data_[idx].value = V();
            data_[idx].occupied = true;
            data_[idx].deleted = false;
            count_++;
        }
        return data_[idx].value;
    }

    bool contains(const K& key) const {
        if (count_ == 0) return false;
        int idx = findIndex(key);
        return idx >= 0 && data_[idx].occupied;
    }

    void erase(const K& key) {
        int idx = findIndex(key);
        if (idx >= 0 && data_[idx].occupied) {
            data_[idx].occupied = false;
            data_[idx].deleted = true;
            count_--;
        }
    }

    V* get(const K& key) {
        if (count_ == 0) return nullptr;
        int idx = findIndex(key);
        return (idx >= 0 && data_[idx].occupied) ? &data_[idx].value : nullptr;
    }

    const V* get(const K& key) const {
        if (count_ == 0) return nullptr;
        int idx = findIndex(key);
        return (idx >= 0 && data_[idx].occupied) ? &data_[idx].value : nullptr;
    }

    int size() const { return count_; }
    bool empty() const { return count_ == 0; }

    void clear() {
        for (int i = 0; i < capacity_; i++) {
            data_[i].occupied = false;
            data_[i].deleted = false;
        }
        count_ = 0;
    }

    // 用于遍历所有 key
    template<typename Func>
    void forEachKey(Func fn) const {
        for (int i = 0; i < capacity_; i++) {
            if (data_[i].occupied) {
                fn(data_[i].key);
            }
        }
    }

    template<typename Func>
    void forEach(Func fn) const {
        for (int i = 0; i < capacity_; i++) {
            if (data_[i].occupied) {
                fn(data_[i].key, data_[i].value);
            }
        }
    }

    DynamicArray<K> keys() const {
        DynamicArray<K> result;
        for (int i = 0; i < capacity_; i++) {
            if (data_[i].occupied) {
                result.push_back(data_[i].key);
            }
        }
        return result;
    }

    DynamicArray<V> values() const {
        DynamicArray<V> result;
        for (int i = 0; i < capacity_; i++) {
            if (data_[i].occupied) {
                result.push_back(data_[i].value);
            }
        }
        return result;
    }

private:
    Entry* data_;
    int capacity_;
    int count_;

    static int nextPow2(int n) {
        int p = 1;
        while (p < n) p *= 2;
        return p;
    }

    void growTo(int newCap) {
        data_ = new Entry[newCap]();
        capacity_ = newCap;
    }

    unsigned int hashKey(const K& key) const {
        return std::hash<K>{}(key);
    }

    int findIndex(const K& key) const {
        unsigned int h = hashKey(key);
        int idx = h & (capacity_ - 1);
        int firstDeleted = -1;

        for (int probe = 0; probe < capacity_; probe++) {
            if (!data_[idx].occupied && !data_[idx].deleted) {
                return (firstDeleted >= 0) ? firstDeleted : idx;
            }
            if (data_[idx].deleted && firstDeleted < 0) {
                firstDeleted = idx;
            }
            if (data_[idx].occupied && data_[idx].key == key) {
                return idx;
            }
            idx = (idx + 1) & (capacity_ - 1);
        }
        return firstDeleted >= 0 ? firstDeleted : 0;
    }

    // 安全的 rehash：分配新表，直接插入，不递归
    void rehash(int newCap) {
        Entry* oldData = data_;
        int oldCap = capacity_;

        data_ = new Entry[newCap]();
        capacity_ = newCap;
        count_ = 0;

        for (int i = 0; i < oldCap; i++) {
            if (oldData[i].occupied) {
                // 直接插入新表，不检查 load factor（不会触发二次 rehash）
                int idx = findIndex(oldData[i].key);
                data_[idx].key = oldData[i].key;
                data_[idx].value = oldData[i].value;
                data_[idx].occupied = true;
                data_[idx].deleted = false;
                count_++;
            }
        }

        delete[] oldData;
    }
};
