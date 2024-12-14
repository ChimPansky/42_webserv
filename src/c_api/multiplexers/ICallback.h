#ifndef WS_C_API_MULTIPLEXERS_I_CALLBACK_H
#define WS_C_API_MULTIPLEXERS_I_CALLBACK_H

#include <unique_ptr.h>

#include <map>

namespace c_api {

enum CallbackType {
    CT_READ = 0x001,
    CT_WRITE = 0x004,
    CT_READWRITE = CT_READ | CT_WRITE
};

// how to:
//   inherit from this class, add necessary fields with refs;
//   call it instead of recv/send/accept/whatever
//   has to be copyable cuz fuck cpp98, so no values inside, refs only
class ICallback {
  protected:
  public:
    // possible returns for errcodes
    // possible args for assert right fd
    // consider changing to void Call()
    virtual void Call(int fd) = 0;
    virtual ~ICallback(){};
};

typedef std::map<int /*fd*/, utils::unique_ptr<ICallback> > FdToCallbackMap;
typedef FdToCallbackMap::const_iterator FdToCallbackMapIt;

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_I_CALLBACK_H
