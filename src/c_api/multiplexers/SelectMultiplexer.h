#ifndef WS_C_API_MULTIPLEXERS_SELECT_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_SELECT_MULTIPLEXER_H

#include "IMultiplexer.h"

namespace c_api {

class SelectMultiplexer : public IMultiplexer {
    virtual int RegisterFd(int, CallbackMode) { return 0; };
    virtual void ReleaseFd(int) {}
    virtual int CheckOnce(const FdToCallbackMap&);
};

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_SELECT_MULTIPLEXER_H
