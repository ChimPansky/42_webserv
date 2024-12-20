#ifndef WS_C_API_MULTIPLEXERS_I_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_I_MULTIPLEXER_H

#include "ICallback.h"

namespace c_api {

enum MultiplexType {
    MT_SELECT = 0,
    MT_POLL = 1,
    MT_EPOLL = 2
};

class IMultiplexer {
  public:
    virtual ~IMultiplexer() {};
    virtual void CheckOnce(const FdToCallbackMap& rd_sockets,
                           const FdToCallbackMap& wr_sockets) = 0;

    virtual bool TryRegisterFd(int /*fd*/, CallbackType /*type*/,
                               const FdToCallbackMap& /*rd_sockets*/,
                               const FdToCallbackMap& /*wr_sockets*/)
    {
        return true;
    };

    virtual void UnregisterFd(int /*fd*/, CallbackType /*type*/,
                              const FdToCallbackMap& /*rd_sockets*/,
                              const FdToCallbackMap& /*wr_sockets*/) {};
};

utils::unique_ptr<IMultiplexer> GetMultiplexer(MultiplexType mx_type, int timeout_ms_);

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_I_MULTIPLEXER_H
