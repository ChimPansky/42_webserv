#include "EpollMultiplexer.h"

#include <fcntl.h>
#include <sys/epoll.h>   // for epoll
#include <unistd.h>      // close

#include "utils/logger.h"

namespace c_api {

EpollMultiplexer::EpollMultiplexer() {
    epoll_fd_ = epoll_create(1);  // argument just for backwards compatibility --> it has no effect, but must be > 0
    if (epoll_fd_ == -1) {
        LOG(FATAL) << "epoll_create failed";
    }
    LOG(DEBUG) << "epoll_create successul --> epoll_fd: " << epoll_fd_;
}

EpollMultiplexer::~EpollMultiplexer() {
    close(epoll_fd_);
}

int EpollMultiplexer::RegisterFd(int fd, CallbackMode mode, const FdToCallbackMap& rd_sockets,
                           const FdToCallbackMap& wr_sockets) {
    LOG(DEBUG) << "\n   RegisterFd " << fd << " for: "
                << (mode == CM_READ ? "read (EPOLLIN)" : mode == CM_WRITE ? "write (EPOLLOUT" : "read/write (EPOLLIN/EPOLLOUT)");
    struct epoll_event ev = {};
    ev.data.fd = fd;
    ev.events = (((mode & CM_READ) ? (int)EPOLLIN : 0) | ((mode & CM_WRITE) ? (int)EPOLLOUT : 0));
    if (rd_sockets.find(fd) != rd_sockets.end() || wr_sockets.find(fd) != wr_sockets.end()) {
        LOG(DEBUG) << "\n Fd is already registered --> use EPOLL_CTL_MOD";
        return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }
    LOG(DEBUG) << "\n Fd is already registered --> use EPOLL_CTL_ADD";
    int res = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    if (res != 0) {
        return res;
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);  // move to diff function, check if needed for select
    return res;
}


int EpollMultiplexer::UnregisterFd(int fd, CallbackMode mode, const FdToCallbackMap& rd_sockets,
                           const FdToCallbackMap& wr_sockets) {
                            // TODOOOOOOOOOOOO (convert mode to events...)
    LOG(DEBUG) << "\n   UnregisterFd " << fd << "; events: " << mode << " been marked for deletion \n"
                "(means he has already sent a request and received a response --> DELETE it from epoll";
    int new_events = GetRegisteredEvents(fd, rd_sockets, wr_sockets) & ~mode;
    int res = 0;
    if (new_events == 0) {
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
    } else {
        struct epoll_event ev = {};
        ev.data.fd = fd;
        ev.events = new_events;
        res = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }
    return res;
}

int EpollMultiplexer::GetRegisteredEvents(int fd, const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets) {
    int reg_events = 0;
    if (rd_sockets.find(fd) != rd_sockets.end()) {
        reg_events |= EPOLLIN;
    }
    if (wr_sockets.find(fd) != wr_sockets.end()) {
        reg_events |= EPOLLOUT;
    }
    return reg_events
}

int EpollMultiplexer::CheckOnce(const FdToCallbackMap& rd_sockets, const FdToCallbackMap& wr_sockets) {

    LOG(DEBUG) << "\n\n---CheckWithEpoll---";
    LOG(DEBUG) << "\n   Iterating over map of monitored sockets which contains \n"
        "   both listeners (aka master sockets aka server sockets) AND \n"
        "   clientsockets (aka communication channel between client and server):";
    // NOTE: i dont get why do u need to modify each time?
    //    either u register every time wit ONESHOT option or register only on add

    // for (FdToCallbackMapIt it = monitored_sockets.begin(); it != monitored_sockets.end(); ++it) {
    //     struct epoll_event ev = {};
    //     ev.events = (it->second->callback_mode() == CM_READ ? EPOLLIN : EPOLLOUT);  // TODO both r and w
    //     ev.data.fd = it->first;
    //     LOG(DEBUG) << "\n   Fd " << it->first << " is already registered in epoll --> MODify (update) its event to: "
    //         << (it->second->callback_mode() == CM_READ ? "read (EPOLLIN)" : "write (EPOLLOUT)");
    //     // epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, it->first, &ev);
    // }
    // LOG(DEBUG) << "\n   Done iterating over map of monitored sockets and updating epoll events\n"
    //     "   --> now call epoll_wait, which will wait for a monitored socket to become available\n"
    //     "   for reading/writing (e.g. a new request is sent by a client). If any becomes available\n"
    //     "   then we get back an array of epoll_events, which are structs that contain \n"
    //     "   an fd and which event(s) its ready for (EPOLLIN/EPOLLOUT)\n";
    struct epoll_event events[EPOLL_MAX_EVENTS];
    int ready_fds = epoll_wait(epoll_fd_, events, EPOLL_MAX_EVENTS, -1);
    if (ready_fds == -1) {
        LOG(ERROR) << "epoll_wait unsuccessful.";
        return 1;
    }
    LOG(DEBUG) << "\n   epoll_wait successful --> " << ready_fds << " fd(s) are ready for read/write --> iterate over them:\n"
    "   For each fd check if it is in our map of monitored sockets and if yes,\n"
    "   then call its callback ICallBack::Call() (in the callback it will be decided\n"
    "   whether to read or write according to ICallBack::callback_mode CM_READ/CM_WRITE)\n";;
    for (int rdy_fd = 0; rdy_fd < ready_fds; rdy_fd++) {
        struct epoll_event& ev = events[rdy_fd];
        if (ev.events & EPOLLIN) { // TODO
            FdToCallbackMapIt it = rd_sockets.find(ev.data.fd);
            if (it != rd_sockets.end()) {
                LOG(DEBUG) << "Fd " << it->first << " is ready for read/write --> call its callback";
                it->second->Call(it->first); // receive/send/accept/whatever
            }
        }
        if (ev.events & EPOLLIN) {
            FdToCallbackMapIt it = wr_sockets.find(ev.data.fd);
            if (it != wr_sockets.end()) {
                LOG(DEBUG) << "Fd " << it->first << " is ready for read/write --> call its callback";
                it->second->Call(it->first); // receive/send/accept/whatever
            }
        }
    }
    return 0;
}


}  // namespace c_api
