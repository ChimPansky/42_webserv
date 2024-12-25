#include <logger.h>

#include "EpollMultiplexer.h"
#include "PollMultiplexer.h"
#include "SelectMultiplexer.h"

namespace c_api {

bool AMultiplexer::TryRegisterFd(int fd, CallbackType type, utils::unique_ptr<ICallback> cb)
{
    if (((type & CT_READ) && (type & CT_WRITE))) {
        throw std::logic_error("One cb cannot be registered for both read and write");
    } else if (!(type & (CT_READ | CT_WRITE))) {
        throw std::logic_error("Cb can only be registered for read or write");
    }
    if (type & CT_READ) {
        if (rd_sockets_.count(fd)) {
            return false;
        }
        rd_sockets_[fd] = cb;
    }
    if (type & CT_WRITE) {
        if (wr_sockets_.count(fd)) {
            return false;
        }
        wr_sockets_[fd] = cb;
    }
    return TryRegisterFdImpl(fd, type);
}

void AMultiplexer::UnregisterFd(int fd, CallbackType type)
{
    int found_type = 0;
    if (type & CT_READ && rd_sockets_.count(fd)) {
        found_type |= CT_READ;
        rd_sockets_.erase(fd);
    }
    if (type & CT_WRITE && wr_sockets_.count(fd)) {
        found_type |= CT_WRITE;
        wr_sockets_.erase(fd);
    }
    if (found_type) {
        UnregisterFdImpl(fd, type);
    }
}

utils::unique_ptr<AMultiplexer> GetMultiplexer(MultiplexType mx_type, int timeout_ms)
{
    switch (mx_type) {
        case MT_SELECT: return utils::unique_ptr<AMultiplexer>(new SelectMultiplexer(timeout_ms));
        case MT_EPOLL: return utils::unique_ptr<AMultiplexer>(new EpollMultiplexer(timeout_ms));
        case MT_POLL: return utils::unique_ptr<AMultiplexer>(new PollMultiplexer(timeout_ms));
        default: LOG(FATAL) << "unknown mx_type"; break;
    }
    return utils::unique_ptr<AMultiplexer>(
        NULL);  // this code is unreacheble it is here to mute warning
}

}  // namespace c_api
