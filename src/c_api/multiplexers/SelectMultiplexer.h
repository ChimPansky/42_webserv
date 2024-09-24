#ifndef WS_C_API_MULTIPLEXERS_SELECT_MULTIPLEXER_H
#define WS_C_API_MULTIPLEXERS_SELECT_MULTIPLEXER_H

#include "IMultiplexer.h"

namespace c_api {

class SelectMultiplexer : public IMultiplexer {
    int CheckOnce(const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets);
    int RegisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                   const FdToCallbackMap& wr_sockets);
    int UnregisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                     const FdToCallbackMap& wr_sockets);
};

}  // namespace c_api

#endif  // WS_C_API_MULTIPLEXERS_SELECT_MULTIPLEXER_H
