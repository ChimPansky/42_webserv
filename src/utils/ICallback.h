#ifndef WS_UTILS_ICALLBACK_H
#define WS_UTILS_ICALLBACK_H

namespace utils {

// how to:
//   inherit from this class, add necessary fields with refs;
//   call it instead of recv/send/accept/whatever
//   has to be copyable cuz fuck cpp98, so no values inside, refs only
class ICallback {
  public:
    // possible returns for errcodes, rm sock from listening, so on
    virtual int call(int fd) = 0;
    virtual ~ICallback() {};
};

}  // namespace utils

#endif  // WS_UTILS_ICALLBACK_H
