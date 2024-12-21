#ifndef WS_UTILS_MAYBE_H
#define WS_UTILS_MAYBE_H

#include <stdexcept>

namespace utils {

template <typename T>
class maybe {
  public:
    maybe() : ok_(false) {}
    maybe(T val) : ok_(true), val_(val) {}

    bool ok() const { return ok_; }

    T& val()
    {
        if (!ok_) {
            throw std::runtime_error("accessing unset value");
        }
        return val_;
    }

    const T& val() const
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

  private:
    bool ok_;
    T val_;
};

}  // namespace utils

#endif  // WS_UTILS_MAYBE_H
