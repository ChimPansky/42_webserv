#include "EpollMultiplexer.h"

#include <stdio.h>      // for perror
#include <sys/epoll.h>  // for epoll
#include <unistd.h>     // close

#include "utils/logger.h"

namespace c_api {

EpollMultiplexer::EpollMultiplexer()
{
    epoll_fd_ = epoll_create(/*deprecated arg, must be gt 0*/ 1);
    if (epoll_fd_ == -1) {
        LOG(FATAL) << "epoll_create failed";
    }
}

EpollMultiplexer::~EpollMultiplexer()
{
    close(epoll_fd_);
}

int EpollMultiplexer::RegisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                                 const FdToCallbackMap& wr_sockets)
{
    struct epoll_event ev = {};
    ev.data.fd = fd;
    ev.events = type;

    int res = 0;
    if (rd_sockets.find(fd) != rd_sockets.end() || wr_sockets.find(fd) != wr_sockets.end()) {
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    } else {
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    }
    if (res != 0) {
        LOG(ERROR) << "epoll_ctl failed: " << fd;
        perror(NULL);
    }
    return res;
    // fcntl(fd, F_SETFL, O_NONBLOCK);  // not allowed as per subject... figure out if needed
}

int EpollMultiplexer::UnregisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                                   const FdToCallbackMap& wr_sockets)
{
    int new_events = GetEventType(fd, rd_sockets, wr_sockets) & ~type;
    int res = 0;
    if (new_events == 0) {
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
    } else {
        struct epoll_event ev = {};
        ev.data.fd = fd;
        ev.events = new_events;
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }
    if (res != 0) {
        LOG(ERROR) << "epoll_ctl failed: " << fd;
        perror(NULL);
    }
    return res;
}

int EpollMultiplexer::GetEventType(int fd, const FdToCallbackMap& rd_sockets,
                                   const FdToCallbackMap& wr_sockets)
{
    int reg_events = 0;
    if (rd_sockets.find(fd) != rd_sockets.end()) {
        reg_events |= EPOLLIN;
    }
    if (wr_sockets.find(fd) != wr_sockets.end()) {
        reg_events |= EPOLLOUT;
    }
    return reg_events;
}

int EpollMultiplexer::CheckOnce(const FdToCallbackMap& rd_sockets,
                                const FdToCallbackMap& wr_sockets)
{
    struct epoll_event events[EPOLL_MAX_EVENTS];
    int ready_fds = epoll_wait(epoll_fd_, events, EPOLL_MAX_EVENTS, -1);
    if (ready_fds == -1) {
        LOG(ERROR) << "epoll_wait unsuccessful.";
        return 1;
    }
    for (int rdy_fd = 0; rdy_fd < ready_fds; rdy_fd++) {
        struct epoll_event& ev = events[rdy_fd];
        if (ev.events & EPOLLIN) {
            FdToCallbackMapIt it = rd_sockets.find(ev.data.fd);
            if (it != rd_sockets.end()) {
                it->second->Call(it->first);  // receive
            }
        }
        if (ev.events & EPOLLOUT) {
            FdToCallbackMapIt it = wr_sockets.find(ev.data.fd);
            if (it != wr_sockets.end()) {
                it->second->Call(it->first);  // send
            }
        }
    }
    return 0;
}

}  // namespace c_api
