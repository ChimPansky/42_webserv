#include "PollMultiplexer.h"

#include <errors.h>
#include <logger.h>
#include <poll.h>

#include <vector>

namespace c_api {

PollMultiplexer::PollMultiplexer(int timeout_ms) : timeout_ms_(timeout_ms)
{}

void PollMultiplexer::CheckOnce(const FdToCallbackMap& rd_sockets,
                                const FdToCallbackMap& wr_sockets)
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

    int num_of_fds = poll(fds.data(), (nfds_t)fds.size(), timeout_ms_);
    if (num_of_fds < 0) {
        LOG_IF(ERROR, errno != EINTR) << "poll failed: " << utils::GetSystemErrorDescr();
        return;
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
}

}  // namespace c_api
