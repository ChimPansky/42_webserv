#ifndef WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H

#include "IMultiplexer.h"

// TODO: does it need to change?
#define EPOLL_MAX_EVENTS 64

namespace c_api {

class EpollMultiplexer : public IMultiplexer {
  public:
    EpollMultiplexer(int timeout_ms);
    ~EpollMultiplexer();
    bool TryRegisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                       const FdToCallbackMap& wr_sockets);
    void UnregisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                      const FdToCallbackMap& wr_sockets);
    void CheckOnce(const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets);

  private:
    int epoll_fd_;
    int timeout_ms_;
};

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H
