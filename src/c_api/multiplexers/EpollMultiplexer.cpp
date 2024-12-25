#include "EpollMultiplexer.h"

#include <errors.h>
#include <logger.h>
#include <sys/epoll.h>  // for epoll
#include <unistd.h>     // close


namespace c_api {

namespace {

int GetRegisteredEventType(int fd, const FdToCallbackMap& rd_sockets_,
                           const FdToCallbackMap& wr_sockets_)
{
    int reg_events = 0;
    if (rd_sockets_.count(fd)) {
        reg_events |= EPOLLIN;
    }
    if (wr_sockets_.count(fd)) {
        reg_events |= EPOLLOUT;
    }
    return reg_events;
}

int32_t CbTypeToEpollEvents(CallbackType type)
{
    return ((type & CT_READ) ? EPOLLIN : 0) | ((type & CT_WRITE) ? EPOLLOUT : 0);
}

}  // namespace

EpollMultiplexer::EpollMultiplexer(int timeout_ms) : timeout_ms_(timeout_ms)
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

bool EpollMultiplexer::TryRegisterFdImpl(int fd, CallbackType type)
{
    struct epoll_event ev = {};
    ev.data.fd = fd;
    ev.events = CbTypeToEpollEvents(type);

    int res = 0;
    if (GetRegisteredEventType(fd, rd_sockets_, wr_sockets_) != 0) {
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    } else {
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    }
    if (res != 0) {
        LOG(ERROR) << "epoll_ctl failed for " << fd << ": " << utils::GetSystemErrorDescr();
    }
    return (res == 0);
}

void EpollMultiplexer::UnregisterFdImpl(int fd, CallbackType type)
{
    uint32_t mod_events =
        GetRegisteredEventType(fd, rd_sockets_, wr_sockets_) & ~CbTypeToEpollEvents(type);
    if (mod_events == 0) {
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
    } else {
        struct epoll_event ev = {};
        ev.data.fd = fd;
        ev.events = mod_events;
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }
}

void EpollMultiplexer::CheckOnce()
{
    struct epoll_event events[EPOLL_MAX_EVENTS];
    int ready_fds = epoll_wait(epoll_fd_, events, EPOLL_MAX_EVENTS, timeout_ms_);
    if (ready_fds < 0) {
        LOG_IF(ERROR, errno != EINTR)
            << "epoll_wait unsuccessful: " << utils::GetSystemErrorDescr();
        return;
    }
    for (int rdy_fd = 0; rdy_fd < ready_fds; rdy_fd++) {
        struct epoll_event& ev = events[rdy_fd];
        if (ev.events & EPOLLIN) {
            FdToCallbackMapIt it = rd_sockets_.find(ev.data.fd);
            if (it != rd_sockets_.end()) {
                it->second->Call(it->first);  // receive
            }
        }
        if (ev.events & EPOLLOUT) {
            FdToCallbackMapIt it = wr_sockets_.find(ev.data.fd);
            if (it != wr_sockets_.end()) {
                it->second->Call(it->first);  // send
            }
        }
    }
}

}  // namespace c_api
