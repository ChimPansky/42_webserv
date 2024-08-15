#include "EventManager.h"

#include <netinet/in.h>  // select

#include <stdexcept>

namespace c_api {

utils::unique_ptr<EventManager> EventManager::_instance(NULL);

// private c'tor
EventManager::EventManager(EventManager::MultiplexType mx_type)
  : _mx_type(mx_type)
{}

void EventManager::init(EventManager::MultiplexType mx_type) {
    if (EventManager::_instance) {
        throw std::runtime_error("event manager was already initialized");
    }
    EventManager::_instance.reset(new EventManager(mx_type));
}

EventManager& EventManager::get() {
    if (!EventManager::_instance) {
        init();
    }
    return (*EventManager::_instance);
}

int EventManager::check_once() {
    if (_mx_type == SELECT) {
        return check_with_select();
    }
    return check_with_select();
}

int EventManager::check_with_select() {
    fd_set select_rd_set;
    fd_set select_wr_set;
    FD_ZERO(&select_rd_set);
    FD_ZERO(&select_wr_set);
    for (SockMapIt it = _rd_sock.begin(); it != _rd_sock.end(); ++it) {
        FD_SET(it->first, &select_rd_set);
    }
    for (SockMapIt it = _wr_sock.begin(); it != _wr_sock.end(); ++it) {
        FD_SET(it->first, &select_rd_set);
    }
    // _rd_sock is never empty as long as at least 1 master socket exist
    int max_fd = _rd_sock.rbegin()->first;
    if (!_wr_sock.empty()) {
        max_fd = std::max(_wr_sock.rbegin()->first, max_fd);
    }
    struct timeval timeout = {10, 0};
    int num_of_fds = select(max_fd, &select_rd_set, &select_wr_set, /* err fds */ NULL, &timeout);
    if (num_of_fds < 0) {
        // select errors or empty set, return error code?
        return 1;
    }
    // iterate over i here cuz call can change callbacks map
    for (int i = 0; i < max_fd; ++i) {
        SockMapIt it;
        if (FD_ISSET(i, &select_rd_set) && ((it = _rd_sock.find(i)) != _rd_sock.end())) {
            it->second->call(i);
        }
        if (FD_ISSET(i, &select_wr_set) && ((it = _wr_sock.find(i)) != _wr_sock.end())) {
            it->second->call(i);
        }
    }
    return 0;
}

int EventManager::register_read_callback(int fd, utils::unique_ptr<utils::ICallback> callback) {
    _rd_sock.insert(std::make_pair(fd, callback));
    return 0;
}

int EventManager::register_write_callback(int fd, utils::unique_ptr<utils::ICallback> callback) {
    _wr_sock.insert(std::make_pair(fd, callback));
    return 0;
}

}  // namespcae c_api
