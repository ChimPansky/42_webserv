#include "SelectMultiplexer.h"

#include <sys/select.h>

namespace c_api {

int SelectMultiplexer::CheckOnce(const FdToCallbackMap& rd_sockets,
                                 const FdToCallbackMap& wr_sockets)
{
    fd_set select_rd_set;
    fd_set select_wr_set;
    FD_ZERO(&select_rd_set);
    FD_ZERO(&select_wr_set);
    for (FdToCallbackMapIt it = rd_sockets.begin(); it != rd_sockets.end(); ++it) {
        FD_SET(it->first, &select_rd_set);
    }
    for (FdToCallbackMapIt it = wr_sockets.begin(); it != wr_sockets.end(); ++it) {
        FD_SET(it->first, &select_wr_set);
    }
    int max_fd = rd_sockets.rbegin()->first;
    if (!wr_sockets.empty()) {
        max_fd = std::max(wr_sockets.rbegin()->first, max_fd);
    }
    // rd_sock_ is never empty as long as at least 1 master socket exist
    struct timeval timeout = {10, 0};
    int num_of_fds =
        select(max_fd + 1, &select_rd_set, &select_wr_set, /* err fds */ NULL, &timeout);
    if (num_of_fds < 0) {
        // select errors or empty set, return error code?
        return 1;
    }
    // iterate over i here cuz call can change callbacks map
    for (int ready_fd = 0; ready_fd <= max_fd; ++ready_fd) {
        if (FD_ISSET(ready_fd, &select_rd_set)) {
            FdToCallbackMapIt it = rd_sockets.find(ready_fd);
            if (it != rd_sockets.end()) {
                it->second->Call(it->first);
            }
        }
        if (FD_ISSET(ready_fd, &select_wr_set)) {
            FdToCallbackMapIt it = wr_sockets.find(ready_fd);
            if (it != wr_sockets.end()) {
                it->second->Call(it->first);
            }
        }
    }
    return 0;
}

int SelectMultiplexer::RegisterFd(int, CallbackType, const FdToCallbackMap&, const FdToCallbackMap&)
{
    return 0;
}

int SelectMultiplexer::UnregisterFd(int, CallbackType, const FdToCallbackMap&,
                                    const FdToCallbackMap&)
{
    return 0;
}

}  // namespace c_api
