// based on Sebastien Rombauts' shared_ptr for c++98 (MIT LICENSE)

#ifndef WS_UTILS_SHARED_PTR_H
#define WS_UTILS_SHARED_PTR_H

#include <algorithm>  // std::swap
#include <stdexcept>

namespace utils {

// no move semantic for shared ptr, so this implementation shold behave like
// std::shared_ptr (check cppref) but not compatible with weak_ptr
template <class T>
class shared_ptr {
  public:
    typedef T element_type;

    shared_ptr(void) throw() : raw_ptr_(NULL), ref_cnt_(NULL) {}

    // destroys object if throw
    explicit shared_ptr(T* p) throw(std::bad_alloc) : ref_cnt_(NULL) { acquire_(p); }

    // Warning : to be used for pointer_cast only ! (does not manage two separate <T> and <U>
    // pointers)
    template <class U>
    shared_ptr(const shared_ptr<U>& rhs, T* p) throw(std::bad_alloc) : ref_cnt_(rhs.ref_cnt_)
    {
        acquire_(p);
    }

    // Copy constructor from another pointer type
    template <class U>
    shared_ptr(const shared_ptr<U>& rhs) : ref_cnt_(rhs.ref_cnt_)
    {
        if ((NULL != rhs.get()) &&
            (0 == rhs.count())) {  // must be coherent : no allocation allowed in this path
            throw std::logic_error("attempt to construct shared_ptr from uncoherent object");
        }
        acquire_(static_cast<typename shared_ptr<T>::element_type*>(rhs.get()));
    }

    shared_ptr(const shared_ptr& rhs) : ref_cnt_(rhs.ref_cnt_)
    {
        if ((NULL != rhs.get()) &&
            (0 == rhs.count())) {  // must be coherent : no allocation allowed in this path
            throw std::logic_error("attempt to construct shared_ptr from uncoherent object");
        }
        acquire_(rhs.raw_ptr_);  // will never throw std::bad_alloc
    }

    // Assignment operator using the copy-and-swap idiom (copy constructor and swap method)
    shared_ptr& operator=(shared_ptr rhs) throw()
    {
        swap(rhs);
        return *this;
    }

    ~shared_ptr() throw() { release_(); }

    void reset() throw() { release_(); }

    void reset(T* p) throw(std::bad_alloc)
    {
        if (p == raw_ptr_) {
            return;
        }
        release_();
        acquire_(p);
    }

    // swap method for the copy-and-swap idiom (copy constructor and swap method)
    void swap(shared_ptr& lhs) throw()
    {
        std::swap(raw_ptr_, lhs.raw_ptr_);
        std::swap(ref_cnt_, lhs.ref_cnt_);
    }

    // reference counter operations :
    operator bool() const throw() { return (0 < count()); }

    bool unique() const throw() { return (1 == count()); }

    long count() const throw()
    {
        long count = 0;
        if (NULL != ref_cnt_) {
            count = *ref_cnt_;
        }
        return count;
    }

    // underlying pointer operations :
    T& operator*() const
    {
        if (NULL == raw_ptr_) {
            throw std::runtime_error("shared_ptr: attempt of dereferencing NULL");
        }
        return *raw_ptr_;
    }

    T* operator->() const
    {
        if (NULL == raw_ptr_) {
            throw std::runtime_error("shared_ptr: attempt of dereferencing NULL");
        }
        return raw_ptr_;
    }

    T* get() const throw() { return raw_ptr_; }

  private:
    // acquire/share the ownership of the raw_ptr_ pointer, initializing the reference counter
    void acquire_(T* p) throw(std::bad_alloc)
    {
        raw_ptr_ = p;
        if (NULL == raw_ptr_) {
            return;
        }
        if (NULL == ref_cnt_) {  // if it is first shared ptr, and the ref_Cnt is not reused
            try {
                ref_cnt_ = new long(1);
            } catch (std::bad_alloc&) {
                delete raw_ptr_;
                raw_ptr_ = NULL;
                ref_cnt_ = NULL;
                throw;
            }
        } else {
            ++(*ref_cnt_);
        }
    }

    // release the ownership of the raw_ptr_ pointer, destroying the object when appropriate
    void release_() throw()
    {
        if (NULL == ref_cnt_) {
            return;
        }
        --(*ref_cnt_);
        if (0 == *ref_cnt_) {  // if it was last shared ptr, delete
            delete raw_ptr_;
            delete ref_cnt_;
        }
        ref_cnt_ = NULL;
        raw_ptr_ = NULL;
    }

  private:
    T* raw_ptr_;
    long* ref_cnt_;
};

template <class T, class U>
bool operator==(const shared_ptr<T>& l, const shared_ptr<U>& r) throw()
{
    return (l.get() == r.get());
}
template <class T, class U>
bool operator!=(const shared_ptr<T>& l, const shared_ptr<U>& r) throw()
{
    return (l.get() != r.get());
}

template <class T, class U>
shared_ptr<T> static_pointer_cast(const shared_ptr<U>& ptr)
{
    return shared_ptr<T>(ptr, static_cast<typename shared_ptr<T>::element_type*>(ptr.get()));
}

template <class T, class U>
shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& ptr)
{
    T* p = dynamic_cast<typename shared_ptr<T>::element_type*>(ptr.get());
    if (NULL != p) {
        return shared_ptr<T>(ptr, p);
    } else {
        return shared_ptr<T>();
    }
}

}  // namespace utils

#endif  // WS_UTILS_SHARED_PTR_H
