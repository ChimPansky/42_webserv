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
    return multiplexer_->CheckOnce(rd_sockets_, wr_sockets_);
}


int EventManager::RegisterCallback(int fd,
                                   CallbackMode mode,
                                   utils::unique_ptr<c_api::ICallback> callback)
{
    if (mode == CM_READ) {
        rd_sockets_[fd] = callback;
    }
    else if (mode == CM_WRITE) {
        wr_sockets_[fd] = callback;
    }
    else {
        LOG(FATAL) << "Unknown callback mode";
    }
    return multiplexer_->RegisterFd(fd, mode);
}

void EventManager::DeleteCallbacksByFd(int fd)
{
    rd_sockets_.erase(fd);
    wr_sockets_.erase(fd);
    multiplexer_->ReleaseFd(fd);
}

}  // namespace c_api
