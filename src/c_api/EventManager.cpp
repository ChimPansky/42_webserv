#include "EventManager.h"

#include <logger.h>

#include <stdexcept>

#include "multiplexers/AMultiplexer.h"

namespace c_api {

utils::unique_ptr<EventManager> EventManager::instance_(NULL);

// private c'tor
EventManager::EventManager(MultiplexType mx_type)
    : multiplexer_(GetMultiplexer(mx_type, kDefaultTimeoutSeconds_() * 1000))
{}

void EventManager::init(MultiplexType mx_type)
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

void EventManager::CheckOnce_()
{
    for (size_t i = 0; i < fds_to_delete_.size(); ++i) {
        multiplexer_->UnregisterFd(fds_to_delete_[i].first, fds_to_delete_[i].second);
    }
    fds_to_delete_.clear();
    multiplexer_->CheckOnce();
}

bool EventManager::TryRegisterCallback_(int fd, CallbackType type,
                                        utils::unique_ptr<c_api::ICallback> callback)
{
    return multiplexer_->TryRegisterFd(fd, type, callback);
}

// TODO: this function is called from d-tor, but not exception safe (may lead to terminate)
//  consider change to commented code below with callback change, or figure out smth else
void EventManager::DeleteCallback_(int fd, CallbackType type)
{
    fds_to_delete_.push_back(std::make_pair(fd, type));
}

// void EventManager::DeleteCallback_(int fd, CallbackType type)
// {
//     if (type & CT_READ) {
//         FdToCallbackMapIt it = rd_sockets_.find(fd);
//         if (it != rd_sockets_.end()) {
//             it->second.reset();
//             multiplexer_->UnregisterFd(fd, CT_READ, rd_sockets_, wr_sockets_);
//         }
//     }
//     if (type & CT_WRITE) {
//         FdToCallbackMapIt it = rd_sockets_.find(fd);
//         if (it != rd_sockets_.end()) {
//             it->second.reset();
//             multiplexer_->UnregisterFd(fd, CT_READ, rd_sockets_, wr_sockets_);
//         }
//     }
// }

}  // namespace c_api
