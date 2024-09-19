#ifndef WS_C_API_MULTIPLEXERS_I_CALLBACK_H
#define WS_C_API_MULTIPLEXERS_I_CALLBACK_H

#include "utils/unique_ptr.h"
#include <map>

namespace c_api {

enum CallbackMode {  // rename o use O_WRITE?
    CM_READ = 1,
    CM_WRITE = 2,
    CM_READWRITE = CM_READ | CM_WRITE
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
    virtual ~ICallback() {};
};

typedef std::map<int /*fd*/, utils::unique_ptr<ICallback> > FdToCallbackMap;
typedef FdToCallbackMap::const_iterator FdToCallbackMapIt;


}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_I_CALLBACK_H
