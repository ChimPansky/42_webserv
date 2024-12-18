#include "EventManager.h"

#include <logger.h>

#include <stdexcept>

#include "multiplexers/IMultiplexer.h"

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
    int res = multiplexer_->CheckOnce(rd_sockets_, wr_sockets_);
    for (size_t i = 0; i < fds_to_delete_.size(); ++i) {
        ClearCallback_(fds_to_delete_[i].first, fds_to_delete_[i].second);
    }
    fds_to_delete_.clear();
    return res;
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

void EventManager::DeleteCallback(int fd, CallbackType type)
{
    if (type & CT_READ && rd_sockets_.find(fd) != rd_sockets_.end()) {
        fds_to_delete_.push_back(std::make_pair(fd, CT_READ));
    }
    if (type & CT_WRITE && wr_sockets_.find(fd) != wr_sockets_.end()) {
        fds_to_delete_.push_back(std::make_pair(fd, CT_WRITE));
    }
}

void EventManager::ClearCallback_(int fd, CallbackType type)
{
    if (type & CT_READ && rd_sockets_.find(fd) != rd_sockets_.end()) {
        if (multiplexer_->UnregisterFd(fd, CT_READ, rd_sockets_, wr_sockets_) != 0) {
            LOG(ERROR) << "Could not unregister read callback for fd: " << fd;
        }
        rd_sockets_.erase(fd);
    }
    if (type & CT_WRITE && wr_sockets_.find(fd) != wr_sockets_.end()) {
        if (multiplexer_->UnregisterFd(fd, CT_WRITE, rd_sockets_, wr_sockets_) != 0) {
            LOG(ERROR) << "Could not unregister write callback for fd: " << fd;
        }
        wr_sockets_.erase(fd);
    }
}

}  // namespace c_api
