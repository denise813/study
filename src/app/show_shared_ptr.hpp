#include <iostream>
#include <memory>

namespace std
{

template<typename T>
class SharedPtr {
private:
    T* ptr;
    std::size_t* count;
 
public:
    SharedPtr(T* p = nullptr) : ptr(p), count(new std::size_t(1)) {}
 
    SharedPtr(const SharedPtr& other) : ptr(other.ptr), count(other.count) {
        if (count != nullptr) {
            ++*count;
        }
    }
 
    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            release();
            ptr = other.ptr;
            count = other.count;
            if (count != nullptr) {
                ++*count;
            }
        }
        return *this;
    }
 
    ~SharedPtr() {
        release();
    }
 
    T& operator*() const {
        return *ptr;
    }
 
    T* operator->() const {
        return ptr;
    }
 
    std::size_t use_count() const {
        if (count != nullptr) {
            return *count;
        }
        return 0;
    }
 
private:
    void release() {
        if (count != nullptr && --*count == 0) {
            delete ptr;
            delete count;
            ptr = nullptr;
            count = nullptr;
        }
    }
};

};
