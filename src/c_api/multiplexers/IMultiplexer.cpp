#include "SelectMultiplexer.h"
#include "EpollMultiplexer.h"

#include "utils/logger.h"

namespace c_api {

utils::shared_ptr<IMultiplexer> GetMultiplexer(MultiplexType mx_type) {
    switch (mx_type) {
        case MT_SELECT: return utils::shared_ptr<IMultiplexer>(new SelectMultiplexer());
        case MT_EPOLL: return utils::shared_ptr<IMultiplexer>(new EpollMultiplexer());
        default: LOG(FATAL) << "unknown mx_type"; break;
    }
    return utils::shared_ptr<IMultiplexer>(NULL);  // this code is unreacheble it is here to mute warning
}

}  // namespace c_api
