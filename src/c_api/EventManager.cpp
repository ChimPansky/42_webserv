#include "EventManager.h"

#include <stdexcept>

#include "utils/logger.h"

namespace c_api {

utils::unique_ptr<EventManager> EventManager::instance_(NULL);

// private c'tor
EventManager::EventManager(MultiplexType mx_type) : multiplexer_(GetMultiplexer(mx_type))
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

int EventManager::CheckOnce()
{
    return multiplexer_->CheckOnce(monitored_sockets_);
}


int EventManager::RegisterCallback(int fd,
                                   CallbackMode mode,
                                   utils::unique_ptr<c_api::ICallback> callback)
{
    monitored_sockets_.insert(std::make_pair(fd, callback));  // TODO adapt fo cb modes. why insert and not monitored_sockets_[fd] = callback?
    return multiplexer_->RegisterFd(fd, mode);
}

void EventManager::DeleteCallbacksByFd(int fd)
{
    monitored_sockets_.erase(fd);  // TODO adapt for modes
    multiplexer_->ReleaseFd(fd);
}

}  // namespace c_api
