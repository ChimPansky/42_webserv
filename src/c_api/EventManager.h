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
    int RegisterWriteCallback(int, utils::unique_ptr<utils::ICallback>);
    void DeleteCallbacksByFd(int fd);

    // all select-poll-epoll logic goes in here
    int check_once();
    static void init(MultiplexType _mx_type = SELECT);
    static EventManager& get();
  private:
    int _CheckWithSelect();
    int _CheckWithPoll();
    int _CheckWithEpoll();
    static utils::unique_ptr<EventManager> _instance;
    MultiplexType _mx_type;
    std::map</* fd */ int, utils::unique_ptr<utils::ICallback> > _rd_sock;
    std::map</* fd */ int, utils::unique_ptr<utils::ICallback> > _wr_sock;
    typedef std::map<int, utils::unique_ptr<utils::ICallback> >::const_iterator SockMapIt;
};

}  // namespace c_api

#endif  // WS_C_API_EVENT_MANAGER_H
