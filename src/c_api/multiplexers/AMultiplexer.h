#ifndef WS_C_API_MULTIPLEXERS_I_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_I_MULTIPLEXER_H

#include "ICallback.h"

namespace c_api {

enum MultiplexType {
    MT_SELECT = 0,
    MT_POLL = 1,
    MT_EPOLL = 2
};

class AMultiplexer {
  public:
    virtual ~AMultiplexer() {};
    virtual void CheckOnce() = 0;

    bool TryRegisterFd(int fd, CallbackType type, utils::unique_ptr<ICallback> cb);

    void UnregisterFd(int fd, CallbackType type);

  protected:
    virtual bool TryRegisterFdImpl(int /*fd*/, CallbackType /*type*/) { return true; }

    virtual void UnregisterFdImpl(int /*fd*/, CallbackType /*effective_type*/) {}

  protected:
    FdToCallbackMap rd_sockets_;  // this contains callbacks for both: listeners & clients
    FdToCallbackMap wr_sockets_;  // this contains callbacks for (write) clients only
};

utils::unique_ptr<AMultiplexer> GetMultiplexer(MultiplexType mx_type, int timeout_ms_);

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_I_MULTIPLEXER_H
