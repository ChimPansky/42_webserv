#include "EventManager.h"

#include <netinet/in.h>  // select
#include <sys/epoll.h>   // for epoll
#include <fcntl.h>       // for fcntl

#include <stdexcept>

#include "../utils/logger.h"

namespace c_api {

utils::unique_ptr<EventManager> EventManager::instance_(NULL);

// private c'tor
EventManager::EventManager(EventManager::MultiplexType mx_type) : mx_type_(mx_type)
{}

void EventManager::init(EventManager::MultiplexType mx_type)
{
    if (EventManager::instance_) {
        throw std::runtime_error("event manager was already initialized");
    }
    EventManager::instance_.reset(new EventManager(mx_type));
}

EventManager& EventManager::get()
{
    if (!EventManager::instance_) {
        throw std::runtime_error("Event manager is not initialised");
    }
    return (*EventManager::instance_);
}

int EventManager::CheckOnce()
{
    switch (mx_type_) {
        case MT_SELECT:
            return CheckWithSelect_();
        case MT_POLL:
            return CheckWithPoll_();
        case MT_EPOLL:
            return CheckWithEpoll_();
    }
    return 1;
}

int EventManager::CheckWithPoll_()
{
    LOG(ERROR) << "CheckWithPoll not implemented";
    return 1;
}

int EventManager::CheckWithEpoll_() {
    LOG(DEBUG) << "CheckWithEpoll";
    struct epoll_event ev;
    struct epoll_event events[EPOLL_MAX_EVENTS];
    int epoll_fd = epoll_create(1); // argument just for backwards compatibility --> its ignored
    //since Linux 2.6.8, but must be > 0 if (epoll_fd == -1) {
    if (epoll_fd == -1) {
        LOG(ERROR) << "epoll_create failed";
        return 1;
    }

    for (SockMapIt it = monitored_sockets_.begin(); it != monitored_sockets_.end(); ++it) {
        ev.data.fd = it->first;
        ev.events = it->second->callback_mode() == CT_READ ? EPOLLIN : EPOLLOUT;
        if (it->second->callback_mode() == CT_READ) {
            ev.events = EPOLLIN;
        }
        else if (it->second->callback_mode() == CT_WRITE) {
            ev.events = EPOLLOUT;
        }
        if (!it->second->added_to_multiplex()) { // fd is not in yet in epoll --> ADD
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, it->first, &ev);
            LOG(DEBUG) << "Adding fd << " << it->first << " to epoll for "
            << (it->second->callback_mode() == CT_READ ? "read (EPOLLIN)" : "write (EPOLLOUT)");
            fcntl(it->first, F_SETFL, O_NONBLOCK);
            it->second->set_added_to_multiplex(true);
        }
        else if (it->second->added_to_multiplex()) { // fd is already in epoll --> either Delete or MODIFY
            if (it->second->callback_mode() == CT_DELETE) {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->first, &ev);
                LOG(DEBUG) << "Deleting fd << " << it->first << " from epoll";
                continue;
            }
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, it->first, &ev);
            LOG(DEBUG) << "Modifying existing fd << " << it->first << " in epoll for "
            << (it->second->callback_mode() == CT_READ ? "read (EPOLLIN)" : "write (EPOLLOUT)");
        }
    }
    int ready_fds = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, -1);
    if (ready_fds == -1) {
        LOG(ERROR) << "epoll_wait unsuccessful.";
        return 1;
    }
    for (int rdy_fd = 0; rdy_fd < ready_fds; rdy_fd++) {
        SockMapIt it;
        if ((it = monitored_sockets_.find(events[rdy_fd].data.fd)) != monitored_sockets_.end()) {
            it->second->Call(it->first); // receive/send/accept/whatever
        }
    }
    // close(epollfd); close when SIGINT?
    return 0;
}

int EventManager::CheckWithSelect_()
{
    fd_set select_rd_set;
    fd_set select_wr_set;
    FD_ZERO(&select_rd_set);
    FD_ZERO(&select_wr_set);
    LOG(DEBUG) << "CheckWithSelect";
    for (SockMapIt it = monitored_sockets_.begin(); it != monitored_sockets_.end(); ++it) {
        if (it->second->callback_mode() == CT_READ) {
            LOG(DEBUG) << "adding to select read set: " << it->first;
            FD_SET(it->first, &select_rd_set);
        }
        else if (it->second->callback_mode() == CT_WRITE) {
            LOG(DEBUG) << "adding to select write set: " << it->first;
            FD_SET(it->first, &select_wr_set);
        }
    }
    // rd_sock_ is never empty as long as at least 1 master socket exist
    int max_fd = monitored_sockets_.rbegin()->first;
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
        SockMapIt it;
        if ((FD_ISSET(ready_fd, &select_rd_set) || FD_ISSET(ready_fd, &select_wr_set)) &&
            ((it = monitored_sockets_.find(ready_fd)) != monitored_sockets_.end())) {
            it->second->Call(ready_fd);  // receive/send/accept/whatever
        }
    }
    LOG(DEBUG) << "CheckWithSelect-> Done iterating over monitored sockets";
    return 0;
}

int EventManager::RegisterCallback(int fd,
                                   utils::unique_ptr<c_api::EventManager::ICallback> callback)
{
    monitored_sockets_.insert(std::make_pair(fd, callback));
    return 0;
}

void EventManager::DeleteCallbacksByFd(int fd)
{
    LOG(DEBUG) << "deleting callback for fd: " << fd;
    monitored_sockets_.erase(fd);
}

}  // namespace c_api
