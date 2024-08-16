#ifndef WS_C_API_EVENT_MANAGER_H
#define WS_C_API_EVENT_MANAGER_H

#include <map>

#include "utils/ICallback.h"
#include "utils/unique_ptr.h"

namespace c_api {

// socker binded to addr
class EventManager {
  public:
    enum MultiplexType {
        SELECT = 0,
        POLL = 1,
        EPOLL = 2
    };
  private:
    EventManager();
    EventManager(const EventManager&);
    EventManager& operator=(const EventManager&);
    EventManager(MultiplexType _mx_type = SELECT);
  public:
    int register_read_callback(int, utils::unique_ptr<utils::ICallback>);
    int register_write_callback(int, utils::unique_ptr<utils::ICallback>);
    // all select-poll-epoll logic goes in here
    int check_once();
    int check_with_select();
    static void init(MultiplexType _mx_type = SELECT);
    static EventManager& get();
  private:
    static utils::unique_ptr<EventManager> _instance;
    std::map</* fd */ int, utils::unique_ptr<utils::ICallback> > _rd_sock;
    std::map</* fd */ int, utils::unique_ptr<utils::ICallback> > _wr_sock;
    MultiplexType _mx_type;
    typedef std::map<int, utils::unique_ptr<utils::ICallback> >::const_iterator SockMapIt;
};

}  // namespace c_api

#endif  // WS_C_API_EVENT_MANAGER_H
