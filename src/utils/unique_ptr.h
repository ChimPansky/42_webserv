// based on Sebastien Rombauts' fake unique_ptr for c++98 (MIT LICENSE)

#ifndef WS_UTILS_UNIQUE_PTR_H
#define WS_UTILS_UNIQUE_PTR_H

#include <cstddef>   // NULL
#include <stdexcept> // or better segfault?

namespace utils {

// owning ptr, calls delete on destruction
// moves underlying raw ptr on copy (move-from state is defined and becomes NULL ptr)
// const unique_ptr<T> leads to ub, cuz const_cast is used in copy-c'tor/assignment
// therefore, never declare const unique_ptr var
template <class T>
class unique_ptr {
  public:
    typedef T element_type;

    // NULL by default
    unique_ptr() throw() : raw_ptr_(NULL)
    {}

    explicit unique_ptr(T* p) throw() : raw_ptr_(p)
    {}

    // Copy-from (rhs) from becomes NULL
    unique_ptr(const unique_ptr& rhs) throw() : raw_ptr_(rhs.raw_ptr_) {
        const_cast<unique_ptr&>(rhs).raw_ptr_ = NULL;  // const-cast to force ownership transfer!
    }

    // copy operator takes value, which nullify precopy rhs
    unique_ptr& operator=(const unique_ptr& rhs) throw() {
        if (this == &rhs) {
            return *this;
        }
        reset(rhs.raw_ptr_);
        const_cast<unique_ptr&>(rhs).raw_ptr_ = NULL;  // const-cast to force ownership transfer!
        return *this;
    }

    inline ~unique_ptr() throw() {
        destroy_();
    }

    inline void reset() throw() {
        destroy_();
    }

    void reset(T* new_ptr) throw() {
        if (raw_ptr_ == new_ptr) {
            return;
        }
        destroy_();
        raw_ptr_ = new_ptr;
    }

    inline operator bool() const throw() {
        return (NULL != raw_ptr_);
    }

    inline T& operator*() const {
        if (raw_ptr_ == NULL) {
            throw std::runtime_error("unique_ptr: attempt of dereferencing NULL");
        }
        return *raw_ptr_;
    }

    inline T* operator->() const {
        if (raw_ptr_ == NULL) {
            throw std::runtime_error("unique_ptr: attempt of dereferencing NULL");
        }
        return raw_ptr_;
    }

    inline T* get() const throw() {
        return raw_ptr_;
    }

  private:
    inline void destroy_() throw() {
        delete raw_ptr_;
        raw_ptr_ = NULL;
    }

  private:
    T* raw_ptr_;
};

template <class T, class U>
inline bool operator==(const unique_ptr<T>& l, const unique_ptr<U>& r) throw() {
    return (l.get() == r.get());
}

template <class T, class U>
inline bool operator!=(const unique_ptr<T>& l, const unique_ptr<U>& r) throw() {
    return (l.get() != r.get());
}


}  // namespace utils

#endif  // WS_UTILS_UNIQUE_PTR_H
