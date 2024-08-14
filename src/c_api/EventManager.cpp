#include "EventManager.h"

#include <netinet/in.h>  // sockaddr_in
// #include <sys/socket.h>  // socket, bind


namespace c_api {

// implement with poll and epoll
int EventManager::check_once() {
    fd_set select_rd_set;
    fd_set select_wr_set;
    FD_ZERO(&select_rd_set);
    FD_ZERO(&select_wr_set);
    for (auto it = _rd_sock.begin(); it != _rd_sock.end(); ++it) {
        FD_SET(it->first, &select_rd_set);
    }
    for (auto it = _wr_sock.begin(); it != _wr_sock.end(); ++it) {
        FD_SET(it->first, &select_rd_set);
    }
    // _rd_sock is never empty as long as at least 1 master socket exist
    int max_fd = _rd_sock.rbegin()->first;
    if (!_wr_sock.empty()) {
        max_fd = std::max(_wr_sock.rbegin()->first, max_fd);
    }
    int num_of_fds = select(max_fd, &select_rd_set, &select_wr_set, /* err fds */ NULL, /* timeout */ NULL);
    if (num_of_fds < 0) {
        // select errors or empty set, return error code?
        return 1;
    }
    // iterate over i here cuz call can change callbacks map
    for (int i = 0; i < max_fd; ++i) {
        std::map<int, utils::ICallback>::iterator it;
        if (FD_ISSET(i, &select_rd_set) && ((it = _rd_sock.find(i)) != _rd_sock.end())) {
            it->second->call(i);
        }
        if (FD_ISSET(i, &select_rd_set) && ((it = _wr_sock.find(i)) != _wr_sock.end())) {
            it->second->call(i);
        }
    }
}

}
