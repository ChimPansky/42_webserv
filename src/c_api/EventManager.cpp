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

int EventManager::RegisterCallback(int fd, CallbackType type,
                                   utils::unique_ptr<c_api::ICallback> callback)
{
    if (!(type & CT_READ) && !(type & CT_WRITE)) {
        LOG(FATAL) << "Unknown callback type";
        return 1;
    }
    if (multiplexer_->RegisterFd(fd, type, rd_sockets_, wr_sockets_) != 0) {
        return 1;
    }
    if (type & CT_READ) {
        rd_sockets_[fd] = callback;
    }
    if (type & CT_WRITE) {
        wr_sockets_[fd] = callback;
    }
    return 0;
}

void EventManager::MarkCallbackForDeletion(int fd, CallbackType type)
{
    fds_to_delete_.push_back(std::make_pair(fd, type));
}

void EventManager::DeleteMarkedCallbacks()
{
    for (size_t i = 0; i < fds_to_delete_.size(); ++i) {
        if (fds_to_delete_[i].second == CT_READ) {
            rd_sockets_.erase(fds_to_delete_[i].first);
            multiplexer_->UnregisterFd(fds_to_delete_[i].first, CT_READ, rd_sockets_, wr_sockets_);
        } else if (fds_to_delete_[i].second == CT_WRITE) {
            wr_sockets_.erase(fds_to_delete_[i].first);
            multiplexer_->UnregisterFd(fds_to_delete_[i].first, CT_WRITE, rd_sockets_, wr_sockets_);
        }
    }
    fds_to_delete_.clear();
}

}  // namespace c_api
