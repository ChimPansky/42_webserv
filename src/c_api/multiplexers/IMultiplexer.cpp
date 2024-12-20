#include <logger.h>

#include "EpollMultiplexer.h"
#include "PollMultiplexer.h"
#include "SelectMultiplexer.h"

namespace c_api {

utils::unique_ptr<IMultiplexer> GetMultiplexer(MultiplexType mx_type, int timeout_ms)
{
    switch (mx_type) {
        case MT_SELECT: return utils::unique_ptr<IMultiplexer>(new SelectMultiplexer(timeout_ms));
        case MT_EPOLL: return utils::unique_ptr<IMultiplexer>(new EpollMultiplexer(timeout_ms));
        case MT_POLL: return utils::unique_ptr<IMultiplexer>(new PollMultiplexer(timeout_ms));
        default: LOG(FATAL) << "unknown mx_type"; break;
    }
    return utils::unique_ptr<IMultiplexer>(
        NULL);  // this code is unreacheble it is here to mute warning
}

}  // namespace c_api
