#include "EventManager.h"

#include <netinet/in.h>  // select
#include <sys/epoll.h>   // for epoll
#include <fcntl.h>       // for fcntl

#include <stdexcept>

#include "../utils/logger.h"

namespace c_api {

utils::unique_ptr<EventManager> EventManager::instance_(NULL);

int EventManager::epoll_fd_ = -1;

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

int EventManager::epoll_fd()
{
    return epoll_fd_;
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
    LOG(DEBUG) << "\n\n---CheckWithEpoll---";
    struct epoll_event ev;
    struct epoll_event events[EPOLL_MAX_EVENTS];

    if (epoll_fd_ == -1) { // first loop iteration --> create epoll fd
        epoll_fd_ = epoll_create(1); // argument just for backwards compatibility --> it has no effect, but must be > 0
        if (epoll_fd_ == -1) {
            LOG(ERROR) << "epoll_create failed";
        }
        else {
            LOG(DEBUG) << "epoll_create successul --> epoll_fd: " << epoll_fd_;
        }
    }

    LOG(DEBUG) << "\n   Iterating over map of monitored sockets which contains \n"
        "   both listeners (aka master sockets aka server sockets) AND \n"
        "   clientsockets (aka communication channel between client and server):";
    for (SockMapIt it = monitored_sockets_.begin(); it != monitored_sockets_.end(); ++it) {
        ev.data.fd = it->first;
        ev.events = it->second->callback_mode() == CM_READ ? EPOLLIN : EPOLLOUT;
        if (it->second->callback_mode() == CM_READ) {
            ev.events = EPOLLIN;
        }
        else if (it->second->callback_mode() == CM_WRITE) {
            ev.events = EPOLLOUT;
        }
        if (!it->second->added_to_multiplex()) { // fd is not in yet in epoll --> ADD
            LOG(DEBUG) << "\n   Fd " << it->first << " is not registered in epoll yet --> ADD to epoll with event: "
                << (it->second->callback_mode() == CM_READ ? "read (EPOLLIN)" : "write (EPOLLOUT)");
            epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, it->first, &ev);
            fcntl(it->first, F_SETFL, O_NONBLOCK);
            it->second->set_added_to_multiplex(true);
        }
        else if (it->second->added_to_multiplex()) { // fd is already in epoll --> either Delete or MODIFY
            if (it->second->callback_mode() == CM_DELETE) {
                LOG(DEBUG) << "\n   Fd " << it->first << " has been marked for deletion \n"
                "(means he has already sent a request and received a response --> DELETE it from epoll";
                epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, it->first, &ev);
                continue;
            }
            LOG(DEBUG) << "\n   Fd " << it->first << " is already registered in epoll --> MODify (update) its event to: "
            << (it->second->callback_mode() == CM_READ ? "read (EPOLLIN)" : "write (EPOLLOUT)");
            epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, it->first, &ev);
        }
    }
    LOG(DEBUG) << "\n   Done iterating over map of monitored sockets and updating epoll events\n"
        "   --> now call epoll_wait, which will wait for a monitored socket to become available\n"
        "   for reading/writing (e.g. a new request is sent by a client). If any becomes available\n"
        "   then we get back an array of epoll_events, which are structs that contain \n"
        "   an fd and which event(s) its ready for (EPOLLIN/EPOLLOUT)\n";
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
        SockMapIt it;
        if ((it = monitored_sockets_.find(events[rdy_fd].data.fd)) != monitored_sockets_.end()) {
            LOG(DEBUG) << "Fd " << it->first << " is ready for read/write --> call its callback";
            it->second->Call(it->first); // receive/send/accept/whatever
        }
    }
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
