#include <logger.h>

#include "EpollMultiplexer.h"
#include "SelectMultiplexer.h"

namespace c_api {

utils::unique_ptr<IMultiplexer> GetMultiplexer(MultiplexType mx_type)
{
    switch (mx_type) {
        case MT_SELECT:
            return utils::unique_ptr<IMultiplexer>(new SelectMultiplexer());
        case MT_EPOLL:
            return utils::unique_ptr<IMultiplexer>(new EpollMultiplexer());
        default:
            LOG(FATAL) << "unknown mx_type";
            break;
    }
    return utils::unique_ptr<IMultiplexer>(
        NULL);  // this code is unreacheble it is here to mute warning
}

}  // namespace c_api
