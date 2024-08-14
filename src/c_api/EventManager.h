#ifndef WS_C_API_EVENT_MANAGER_H
#define WS_C_API_EVENT_MANAGER_H

#include <memory>
#include <map>
#include "../utils/ICallback.h"


namespace c_api {

// socker binded to addr
class EventManager {
  private:
    EventManager(const EventManager&);
    EventManager& operator=(const EventManager&);
    enum MultiplexType {
        SELECT = 0,
        POLL = 1,
        EPOLL = 2
    };
  public:
    EventManager(MultiplexType _mx_type = SELECT);
    int register_read_callback(int, utils::shared_ptr<utils::ICallback>);
    int register_write_callback(int, utils::shared_ptr<utils::ICallback>);
    // all select-poll-epoll logic goes in here
    int check_once();
  private:
    std::map</* fd */ int, utils::shared_ptr<utils::ICallback> > _rd_sock;
    std::map</* fd */ int, utils::shared_ptr<utils::ICallback> > _wr_sock;
    MultiplexType _mx_type;
};

}  // namespace c_api

#endif  // WS_C_API_EVENT_MANAGER_H
