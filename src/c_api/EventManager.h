#ifndef WS_C_API_EVENT_MANAGER_H
#define WS_C_API_EVENT_MANAGER_H

#include <vector>

#include "multiplexers/ICallback.h"
#include "multiplexers/IMultiplexer.h"

#include <unique_ptr.h>

namespace c_api {

// socker binded to addr
class EventManager {
  private:
    EventManager();
    EventManager(const EventManager&);
    EventManager& operator=(const EventManager&);
    EventManager(MultiplexType mx_type_);

  public:
    static void init(MultiplexType mx_type_);
    static EventManager& get();
    // all select-poll-epoll logic goes in here
    int CheckOnce();
    int RegisterCallback(int fd, CallbackType type, utils::unique_ptr<ICallback>);
    void DeleteCallback(int fd, CallbackType type);
    void  CloseAllFds();


  private:
    void ClearCallback_(int fd, CallbackType type);
    static utils::unique_ptr<EventManager> instance_;
    utils::unique_ptr<IMultiplexer> multiplexer_;
    FdToCallbackMap rd_sockets_;  // this contains callbacks for both: listeners & clients
    FdToCallbackMap wr_sockets_;  // this contains callbacks for (write) clients only
    std::vector<std::pair<int, CallbackType> > fds_to_delete_;  // to delete after CheckOnce
};

}  // namespace c_api

#endif  // WS_C_API_EVENT_MANAGER_H
