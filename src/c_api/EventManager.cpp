#include "EventManager.h"

#include <stdexcept>

#include "multiplexers/IMultiplexer.h"
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

utils::shared_ptr<IMultiplexer> EventManager::multiplexer() const
{
    return multiplexer_;
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

void EventManager::MarkCallbackForDeletion(int fd, CallbackMode mode) {
    fds_to_delete_.push_back(std::make_pair(fd, mode));
}

void EventManager::DeleteFinishedCallbacks() {
    for (size_t i = 0; i < fds_to_delete_.size(); ++i) {
        if (fds_to_delete_[i].second == CM_READ) {
            rd_sockets_.erase(fds_to_delete_[i].first);
        }
        else if (fds_to_delete_[i].second == CM_WRITE) {
            wr_sockets_.erase(fds_to_delete_[i].first);
        }
        multiplexer_->ReleaseFd(fds_to_delete_[i].first);
    }
    fds_to_delete_.clear();
}

}  // namespace c_api
