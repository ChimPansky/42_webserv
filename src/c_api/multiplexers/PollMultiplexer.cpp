#include "PollMultiplexer.h"

#include <logger.h>
#include <poll.h>

#include <cstring>
#include <vector>

namespace c_api {

int PollMultiplexer::CheckOnce(const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets)
{
    std::vector<struct pollfd> fds;

    for (FdToCallbackMapIt it = rd_sockets.begin(); it != rd_sockets.end(); ++it) {
        struct pollfd fd_struct = {it->first, POLLIN, /*revents*/ 0};
        fds.push_back(fd_struct);
    }
    for (FdToCallbackMapIt it = wr_sockets.begin(); it != wr_sockets.end(); ++it) {
        struct pollfd fd_struct = {it->first, POLLOUT, /*revents*/ 0};
        fds.push_back(fd_struct);
    }

    int timeout_ms = 10000;  // 10s

    int num_of_fds = poll(fds.data(), (nfds_t)fds.size(), timeout_ms);
    if (num_of_fds < 0) {
        LOG(ERROR) << "poll failed: " << std::strerror(errno);
        return 1;
    }
    for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
        if (it->revents & POLLIN) {
            FdToCallbackMapIt sockets_it = rd_sockets.find(it->fd);
            if (sockets_it != rd_sockets.end()) {
                sockets_it->second->Call(sockets_it->first);
            }
        }
        if (it->revents & POLLOUT) {
            FdToCallbackMapIt sockets_it = wr_sockets.find(it->fd);
            if (sockets_it != wr_sockets.end()) {
                sockets_it->second->Call(sockets_it->first);
            }
        }
    }
    return 0;
}

int PollMultiplexer::RegisterFd(int, CallbackType, const FdToCallbackMap&, const FdToCallbackMap&)
{
    return 0;
}

int PollMultiplexer::UnregisterFd(int, CallbackType, const FdToCallbackMap&, const FdToCallbackMap&)
{
    return 0;
}

}  // namespace c_api
