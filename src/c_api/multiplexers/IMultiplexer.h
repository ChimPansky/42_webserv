#ifndef WS_C_API_MULTIPLEXERS_I_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_I_MULTIPLEXER_H

#include "utils/unique_ptr.h"
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
    virtual int CheckOnce(const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets) = 0;
    virtual int RegisterFd(int fd, CallbackMode mode) = 0;
    virtual void ReleaseFd(int fd) = 0;
};

utils::unique_ptr<IMultiplexer> GetMultiplexer(MultiplexType mx_type);

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_I_MULTIPLEXER_H
