#ifndef WS_UTILS_MAYBE_H
#define WS_UTILS_MAYBE_H

#include <stdexcept>

namespace utils {

struct maybe_not {};

template <typename T>
class maybe {
  public:
    maybe() : ok_(false) {}
    maybe(maybe_not) : ok_(false) {}
    maybe(const T& val) : ok_(true), val_(val) {}

    bool ok() const { return ok_; }
    operator bool() const { return ok_; }

    T& value()
    {
        if (!ok_) {
            throw std::runtime_error("accessing unset value");
        }
        return val_;
    }

    const T& value() const
    {
        if (!ok_) {
            throw std::runtime_error("accessing unset value");
        }
        return val_;
    }

    T& operator*()
    {
        if (!ok_) {
            throw std::runtime_error("accessing unset value");
        }
        return val_;
    }

    const T& operator*() const
    {
        if (!ok_) {
            throw std::runtime_error("accessing unset value");
        }
        return val_;
    }

    T* operator->()
    {
        if (!ok_) {
            throw std::runtime_error("accessing unset value");
        }
        return &val_;
    }

    const T* operator->() const
    {
        if (!ok_) {
            throw std::runtime_error("accessing unset value");
        }
        return &val_;
    }

    bool operator==(const maybe& rhs) const { return (ok_ == rhs.ok_ && val_ == rhs.val_); }

    void reset() { ok_ = false; }

    void reset(const T& val)
    {
        ok_ = true;
        val_ = val;
    }

  private:
    bool ok_;
    T val_;
};

}  // namespace utils

#endif  // WS_UTILS_MAYBE_H
