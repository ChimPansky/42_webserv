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
    if (((type & CT_READ) && rd_sockets_.count(fd)) ||
        ((type & CT_WRITE) && wr_sockets_.count(fd))) {
        LOG(ERROR) << "fd " << fd << " was already registered with mode: " << type;
        return false;
    }
    if (!TryRegisterFdImpl(fd, type)) {
        return false;
    }
    if (type & CT_READ) {
        rd_sockets_[fd] = cb;
    } else if (type & CT_WRITE) {
        wr_sockets_[fd] = cb;
    }
    return true;
}

void AMultiplexer::UnregisterFd(int fd, CallbackType type)
{
    int found_type = 0;
    if (type & CT_READ && rd_sockets_.count(fd)) {
        found_type |= CT_READ;
    }
    if (type & CT_WRITE && wr_sockets_.count(fd)) {
        found_type |= CT_WRITE;
    }
    if (found_type) {
        UnregisterFdImpl(fd, type);
    }
    if (found_type & CT_READ) {
        rd_sockets_.erase(fd);
    }
    if (found_type & CT_WRITE) {
        wr_sockets_.erase(fd);
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
