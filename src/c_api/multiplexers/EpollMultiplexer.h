#ifndef WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H

#include "AMultiplexer.h"

// TODO: does it need to change?
#define EPOLL_MAX_EVENTS 64

namespace c_api {

class EpollMultiplexer : public AMultiplexer {
  public:
    EpollMultiplexer(int timeout_ms);
    ~EpollMultiplexer();

    bool TryRegisterFdImpl(int fd, CallbackType type);
    void UnregisterFdImpl(int fd, CallbackType type);
    void CheckOnce();

  private:
    int epoll_fd_;
    int timeout_ms_;
};

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_EPOLL_MULTIPLEXER_H
