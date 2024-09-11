#ifndef WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H

#include "IMultiplexer.h"

#define EPOLL_MAX_EVENTS 64

namespace c_api {

class EpollMultiplexer : public IMultiplexer {
  public:
    EpollMultiplexer();
    ~EpollMultiplexer();
    virtual int RegisterFd(int fd, CallbackMode);
    virtual void ReleaseFd(int fd);
    virtual int CheckOnce(const FdToCallbackMap&);
  private:
    int epoll_fd_;
};

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H
