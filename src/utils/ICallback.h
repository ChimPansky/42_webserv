#ifndef WS_UTILS_ICALLBACK_H
#define WS_UTILS_ICALLBACK_H

namespace utils {

// how to:
//   inherit from this class, add necessary fields with refs;
//   call it instead of recv/send/accept/whatever
//   has to be copyable cuz fuck cpp98, so no values inside, refs only
class ICallback {
  public:
    // possible returns for errcodes
    // possible args for assert right fd
    // consider changing to void Call()
    virtual void Call(int fd) = 0;
    virtual ~ICallback(){};
};

}  // namespace utils

#endif  // WS_UTILS_ICALLBACK_H
