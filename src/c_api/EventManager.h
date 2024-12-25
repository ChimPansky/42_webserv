#ifndef WS_C_API_EVENT_MANAGER_H
#define WS_C_API_EVENT_MANAGER_H

#include <unique_ptr.h>

#include <vector>

#include "multiplexers/AMultiplexer.h"
#include "multiplexers/ICallback.h"

namespace c_api {

// socker binded to addr
class EventManager {
  private:
    EventManager();
    EventManager(const EventManager&);
    EventManager& operator=(const EventManager&);

  private:
    EventManager(MultiplexType mx_type_);

    bool TryRegisterCallback_(int fd, CallbackType type, utils::unique_ptr<ICallback>);
    void DeleteCallback_(int fd, CallbackType type);
    void CheckOnce_();

  public:
    static void init(MultiplexType mx_type_);
    static EventManager& get();

    static inline int kDefaultTimeoutSeconds_() { return 10; }

    // all select-poll-epoll logic goes in here
    static inline void CheckOnce()
    {
        if (instance_) {
            instance_->CheckOnce_();
        }
    }

    // Cannot replace existing cb!
    static inline bool TryRegisterCallback(int fd, CallbackType type,
                                           utils::unique_ptr<ICallback> cb)
    {
        if (instance_) {
            return instance_->TryRegisterCallback_(fd, type, cb);
        }
        return false;
    }

    // Deletes with delay, after before next CheckOnce
    static inline void DeleteCallback(int fd, CallbackType type = CT_READWRITE) throw()
    {
        if (instance_) {
            instance_->DeleteCallback_(fd, type);
        }
    }

  private:
    static utils::unique_ptr<EventManager> instance_;
    utils::unique_ptr<AMultiplexer> multiplexer_;
    std::vector<std::pair<int, CallbackType> > fds_to_delete_;  // to delete after CheckOnce
};

}  // namespace c_api

#endif  // WS_C_API_EVENT_MANAGER_H
