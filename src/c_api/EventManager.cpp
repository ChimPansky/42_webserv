#include "EventManager.h"

#include <netinet/in.h>  // select
#include <sys/select.h>

#include <stdexcept>

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
    if (mx_type_ == MT_SELECT) {
        return CheckWithSelect_();
    }
    return CheckWithSelect_();
}

int EventManager::CheckWithSelect_()
{
    fd_set select_rd_set;
    fd_set select_wr_set;
    FD_ZERO(&select_rd_set);
    FD_ZERO(&select_wr_set);
    for (SockMapIt it = rd_sock_.begin(); it != rd_sock_.end(); ++it) {
        FD_SET(it->first, &select_rd_set);
    }
    for (SockMapIt it = wr_sock_.begin(); it != wr_sock_.end(); ++it) {
        FD_SET(it->first, &select_wr_set);
    }
    // rd_sock_ is never empty as long as at least 1 master socket exist
    int max_fd = rd_sock_.rbegin()->first;
    if (!wr_sock_.empty()) {
        max_fd = std::max(wr_sock_.rbegin()->first, max_fd);
    }
    struct timeval timeout = {10, 0};
    int num_of_fds =
        select(max_fd + 1, &select_rd_set, &select_wr_set, /* err fds */ NULL, &timeout);
    if (num_of_fds < 0) {
        // select errors or empty set, return error code?
        return 1;
    }
    // iterate over i here cuz call can change callbacks map
    for (int i = 0; i <= max_fd; ++i) {
        SockMapIt it;
        if (FD_ISSET(i, &select_rd_set) && ((it = rd_sock_.find(i)) != rd_sock_.end())) {
            it->second->Call(i);
        }
        if (FD_ISSET(i, &select_wr_set) && ((it = wr_sock_.find(i)) != wr_sock_.end())) {
            it->second->Call(i);
        }
    }
    return 0;
}

int EventManager::RegisterReadCallback(int fd, utils::unique_ptr<c_api::EventManager::ICallback> callback)
{
    rd_sock_.insert(std::make_pair(fd, callback));
    return 0;
}

int EventManager::RegisterWriteCallback(int fd, utils::unique_ptr<c_api::EventManager::ICallback> callback)
{
    wr_sock_.insert(std::make_pair(fd, callback));
    return 0;
}

void EventManager::DeleteCallbacksByFd(int fd, CallbackType type)
{
    if (type & CT_READ) {
        rd_sock_.erase(fd);
    }
    if (type & CT_WRITE) {
        wr_sock_.erase(fd);
    }
}

}  // namespace c_api
