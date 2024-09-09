#include "SelectMultiplexer.h"

#include <sys/select.h>

#include <map>

#include "utils/logger.h"

namespace c_api {

int SelectMultiplexer::CheckOnce(const std::map</* fd */ int, utils::unique_ptr<ICallback> >& monitored_sockets) {
    fd_set select_rd_set;
    fd_set select_wr_set;
    FD_ZERO(&select_rd_set);
    FD_ZERO(&select_wr_set);
    LOG(DEBUG) << "CheckWithSelect";
    for (FdToCallbackMapIt it = monitored_sockets.begin(); it != monitored_sockets.end(); ++it) {
        if (it->second->callback_mode() == CM_READ) {
            LOG(DEBUG) << "adding to select read set: " << it->first;
            FD_SET(it->first, &select_rd_set);
        }
        else if (it->second->callback_mode() == CM_WRITE) {
            LOG(DEBUG) << "adding to select write set: " << it->first;
            FD_SET(it->first, &select_wr_set);
        }
    }
    // rd_sock_ is never empty as long as at least 1 master socket exist
    int max_fd = monitored_sockets.rbegin()->first;
    struct timeval timeout = {10, 0};
    int num_of_fds =
        select(max_fd + 1, &select_rd_set, &select_wr_set, /* err fds */ NULL, &timeout);
    if (num_of_fds < 0) {
        // select errors or empty set, return error code?
        return 1;
    }
    // iterate over i here cuz call can change callbacks map
    for (int ready_fd = 0; ready_fd <= max_fd; ++ready_fd) {
        LOG(DEBUG) << "CheckWithSelect-> Iterating over monitored sockets. Current fd: " << ready_fd;
        FdToCallbackMapIt it;
        if ((FD_ISSET(ready_fd, &select_rd_set) || FD_ISSET(ready_fd, &select_wr_set)) &&
            ((it = monitored_sockets.find(ready_fd)) != monitored_sockets.end())) {
            it->second->Call(ready_fd);  // receive/send/accept/whatever
        }
    }
    LOG(DEBUG) << "CheckWithSelect-> Done iterating over monitored sockets";
    return 0;
}

}  // namespace c_api
