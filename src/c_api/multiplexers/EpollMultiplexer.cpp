#include "EpollMultiplexer.h"

#include <fcntl.h>
#include <sys/epoll.h>  // for epoll
#include <unistd.h>     // close

#include "utils/logger.h"

namespace c_api {

EpollMultiplexer::EpollMultiplexer()
{
    epoll_fd_ = epoll_create(
        1);  // argument just for backwards compatibility --> it has no effect, but must be > 0
    if (epoll_fd_ == -1) {
        LOG(FATAL) << "epoll_create failed";
    }
    LOG(DEBUG) << "epoll_create successul --> epoll_fd: " << epoll_fd_;
}

EpollMultiplexer::~EpollMultiplexer()
{
    close(epoll_fd_);
}

int EpollMultiplexer::RegisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                                 const FdToCallbackMap& wr_sockets)
{
    LOG(DEBUG) << "\n   RegisterFd " << fd << " for: "
               << (type == CT_READ    ? "read (EPOLLIN)"
                   : type == CT_WRITE ? "write (EPOLLOUT"
                                      : "read/write (EPOLLIN/EPOLLOUT)");
    struct epoll_event ev = {};
    ev.data.fd = fd;
    ev.events = type;

    if (rd_sockets.find(fd) != rd_sockets.end() || wr_sockets.find(fd) != wr_sockets.end()) {
        LOG(DEBUG) << "\n Fd is already registered --> use EPOLL_CTL_MOD";
        return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }
    LOG(DEBUG) << "\n Fd is not yet registered --> use EPOLL_CTL_ADD";
    int res = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    if (res != 0) {
        return res;
    }
    // fcntl(fd, F_SETFL, O_NONBLOCK);  // not allowed as per subject... figure out how to do it
    // without...
    return res;
}

int EpollMultiplexer::UnregisterFd(int fd, CallbackType type, const FdToCallbackMap& rd_sockets,
                                   const FdToCallbackMap& wr_sockets)
{
    LOG(DEBUG) << "\n   UnregisterFd " << fd << "; Event to unregister: " << type;
    int new_events = GetEventType(fd, rd_sockets, wr_sockets) & ~type;
    int res = 0;
    if (new_events == 0) {
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
        LOG(DEBUG) << "UnregisterFd " << fd << "; Use EPOLL_CTL_DEL";
    } else {
        struct epoll_event ev = {};
        ev.data.fd = fd;
        ev.events = new_events;
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
        LOG(DEBUG) << "UnregisterFd " << fd << "; Use EPOLL_CTL_MOD with " << new_events;
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
    LOG(DEBUG) << "\n\n---CheckWithEpoll---";
    LOG(DEBUG) << "\n   Iterating over map of monitored sockets which contains \n"
                  "   both listeners (aka master sockets aka server sockets) AND \n"
                  "   clientsockets (aka communication channel between client and server):";
    struct epoll_event events[EPOLL_MAX_EVENTS];
    LOG(DEBUG) << "\nEPOLL_WAIT...";
    int ready_fds = epoll_wait(epoll_fd_, events, EPOLL_MAX_EVENTS, -1);
    if (ready_fds == -1) {
        LOG(ERROR) << "epoll_wait unsuccessful.";
        return 1;
    }
    LOG(DEBUG)
        << "\n   epoll_wait successful --> " << ready_fds
        << " fd(s) are ready for read/write --> iterate over them:\n"
           "   For each fd check if it is in our map of read or write sockets and if yes,\n"
           "   then call its callback \n";
    ;
    for (int rdy_fd = 0; rdy_fd < ready_fds; rdy_fd++) {
        struct epoll_event& ev = events[rdy_fd];
        if (ev.events & EPOLLIN) {
            FdToCallbackMapIt it = rd_sockets.find(ev.data.fd);
            if (it != rd_sockets.end()) {
                LOG(DEBUG) << "Fd " << it->first
                           << " is ready for read/receive --> call its callback";
                it->second->Call(it->first);  // receive
            }
        }
        if (ev.events & EPOLLOUT) {
            FdToCallbackMapIt it = wr_sockets.find(ev.data.fd);
            if (it != wr_sockets.end()) {
                LOG(DEBUG) << "Fd " << it->first
                           << " is ready for write/send --> call its callback";
                it->second->Call(it->first);  // send
            }
        }
    }
    return 0;
}

}  // namespace c_api
