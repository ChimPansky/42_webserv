#ifndef WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H

#include "IMultiplexer.h"

#define EPOLL_MAX_EVENTS 64

namespace c_api {

class EpollMultiplexer : public IMultiplexer {
  public:
    EpollMultiplexer();
    ~EpollMultiplexer();
    virtual int RegisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                           const FdToCallbackMap& wr_sockets);
    virtual int UnregisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                             const FdToCallbackMap& wr_sockets);
    virtual int CheckOnce(const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets);

  private:
    int epoll_fd_;
    int GetEventType(int fd, const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets);
};

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H
