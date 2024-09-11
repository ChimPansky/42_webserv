#ifndef WS_C_API_EVENT_MANAGER_H
#define WS_C_API_EVENT_MANAGER_H

#include "utils/unique_ptr.h"
#include "c_api/multiplexers/IMultiplexer.h"

namespace c_api {

// socker binded to addr
class EventManager {
  private:
    EventManager();
    EventManager(const EventManager&);
    EventManager& operator=(const EventManager&);
    EventManager(MultiplexType mx_type_);

  public:
    // use return to indicate error, eg, callback for fd already registered?
    int RegisterCallback(int fd, CallbackMode mode, utils::unique_ptr<ICallback>);
    void DeleteCallbacksByFd(int fd);

    // all select-poll-epoll logic goes in here
    int CheckOnce();
    static void init(MultiplexType mx_type_);
    static EventManager& get();

  private:
    int CheckWithSelect_();
    int CheckWithPoll_();
    int CheckWithEpoll_();
    static utils::unique_ptr<EventManager> instance_;
    utils::unique_ptr<IMultiplexer> multiplexer_;
    FdToCallbackMap monitored_sockets_;  // this contains callbacks for both: listeners (master sockets aka server socket) and clients...
};

}  // namespace c_api

#endif  // WS_C_API_EVENT_MANAGER_H
