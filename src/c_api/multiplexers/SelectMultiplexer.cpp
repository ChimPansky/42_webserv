#include "SelectMultiplexer.h"

#include <logger.h>
#include <sys/select.h>

#include <cstring>

namespace c_api {

SelectMultiplexer::SelectMultiplexer(int timeout_ms) : timeout_(NULL), timeout_descr_()
{
    if (timeout_ms >= 0) {
        timeout_descr_.tv_sec = timeout_ms / 1000;
        timeout_descr_.tv_usec = (timeout_ms % 1000) * 1000;
        timeout_ = &timeout_descr_;
    }
}

void SelectMultiplexer::CheckOnce(const FdToCallbackMap& rd_sockets,
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

    int max_fd = (!rd_sockets.empty() ? rd_sockets.rbegin()->first : 0);
    if (!wr_sockets.empty()) {
        max_fd = std::max(wr_sockets.rbegin()->first, max_fd);
    }
    if (max_fd > kMaxSelectFds_()) {
        max_fd = kMaxSelectFds_();
    }

    int num_of_fds = select(max_fd + 1, &select_rd_set, &select_wr_set, /*err_fds*/ NULL, timeout_);
    if (num_of_fds < 0) {
        LOG_IF(ERROR, errno != EINTR) << "select failed: " << strerror(errno);
        return;
    }

    // iterate over fds here cuz call can change callbacks map
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
}

}  // namespace c_api
