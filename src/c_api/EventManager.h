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
        MT_SELECT = 0,
        MT_POLL = 1,
        MT_EPOLL = 2
    };
    enum CallbackType {
        CT_READ = 1,
        CT_WRITE = 2,
        CT_ANY = 3  // CT_ANY == CT_READ | CT_WRITE
    };

  private:
    EventManager();
    EventManager(const EventManager&);
    EventManager& operator=(const EventManager&);
    EventManager(MultiplexType _mx_type);

  public:
    // use return to indicate error, eg, callback for fd already registered?
    int RegisterReadCallback(int, utils::unique_ptr<utils::ICallback>);
    int RegisterWriteCallback(int, utils::unique_ptr<utils::ICallback>);
    void DeleteCallbacksByFd(int fd, CallbackType cb_type = CT_ANY);

    // all select-poll-epoll logic goes in here
    int CheckOnce();
    static void init(MultiplexType _mx_type);
    static EventManager& get();

  private:
    int CheckWithSelect_();
    int CheckWithPoll_();
    int CheckWithEpoll_();
    static utils::unique_ptr<EventManager> instance_;
    MultiplexType _mx_type;
    std::map</* fd */ int, utils::unique_ptr<utils::ICallback> > _rd_sock;
    std::map</* fd */ int, utils::unique_ptr<utils::ICallback> > _wr_sock;
    typedef std::map<int, utils::unique_ptr<utils::ICallback> >::const_iterator SockMapIt;
};

}  // namespace c_api

#endif  // WS_C_API_EVENT_MANAGER_H
