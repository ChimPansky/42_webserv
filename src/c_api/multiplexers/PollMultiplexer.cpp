#include "PollMultiplexer.h"

#include <poll.h>

#include <vector>

namespace c_api {

int PollMultiplexer::CheckOnce(const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets)
{
    std::vector<struct pollfd> fds;
    nfds_t nb_of_fds;

    for (FdToCallbackMapIt it = rd_sockets.begin(); it != rd_sockets.end(); ++it) {
        struct pollfd fd_struct = {it->first, POLLIN, 0};
        fds.push_back(fd_struct);
    }
    for (FdToCallbackMapIt it = wr_sockets.begin(); it != wr_sockets.end(); ++it) {
        struct pollfd fd_struct = {it->first, POLLOUT, 0};
        fds.push_back(fd_struct);
    }

    nb_of_fds = fds.size();
    int timeout = 10000;  // ms, or 10 sec

    int num_of_fds = poll(fds.data(), nb_of_fds, timeout);
    if (num_of_fds < 0) {
        // poll error or empty array of structs
        return 1;
    }
    for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it) {
        FdToCallbackMapIt sockets_it = rd_sockets.find(it->fd);
        if ((it->revents & POLLIN) && sockets_it != rd_sockets.end()) {
            sockets_it->second->Call(sockets_it->first);
        }
        sockets_it = wr_sockets.find(it->fd);
        if ((it->revents & POLLOUT) && sockets_it != wr_sockets.end()) {
            sockets_it->second->Call(sockets_it->first);
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
