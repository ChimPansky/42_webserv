#ifndef WS_C_API_EVENT_MANAGER_H
#define WS_C_API_EVENT_MANAGER_H

#include <map>

#include "utils/unique_ptr.h"

namespace c_api {

#define EPOLL_MAX_EVENTS 64

// socker binded to addr
class EventManager {
  public:
    enum MultiplexType {
        MT_SELECT = 0,
        MT_POLL = 1,
        MT_EPOLL = 2
    };
    enum CallbackMode {
        CM_READ = 1,
        CM_WRITE = 2,
        CM_DELETE = 4,
        CM_ANY = 7  // CM_ANY == CM_READ | CM_WRITE | CM_DELETE
    };
    // how to:
    //   inherit from this class, add necessary fields with refs;
    //   call it instead of recv/send/accept/whatever
    //   has to be copyable cuz fuck cpp98, so no values inside, refs only
    class ICallback {
      public:
        // possible returns for errcodes
        // possible args for assert right fd
        // consider changing to void Call()
        virtual void Call(int fd) = 0;
        virtual CallbackMode callback_mode() = 0;    // read/write/delete
        virtual bool added_to_multiplex() = 0;    // true when already added to select/poll/epoll fd-set
        virtual void set_added_to_multiplex(bool) = 0;
        virtual ~ICallback(){};
    };

  private:
    EventManager();
    EventManager(const EventManager&);
    EventManager& operator=(const EventManager&);
    EventManager(MultiplexType mx_type_);

  public:
    // use return to indicate error, eg, callback for fd already registered?
    int RegisterCallback(int, utils::unique_ptr<ICallback>);
    void DeleteCallbacksByFd(int fd);

    // all select-poll-epoll logic goes in here
    int CheckOnce();
    static void init(MultiplexType mx_type_);
    static EventManager& get();
    static int epoll_fd();

  private:
    int CheckWithSelect_();
    int CheckWithPoll_();
    int CheckWithEpoll_();
    static utils::unique_ptr<EventManager> instance_;
    static int epoll_fd_;
    MultiplexType mx_type_;
    std::map</* fd */ int, utils::unique_ptr<ICallback> > monitored_sockets_;  // this contains callbacks for both: listeners (master sockets aka server socket) and clients...
    typedef std::map<int, utils::unique_ptr<ICallback> >::const_iterator SockMapIt;
};

}  // namespace c_api

#endif  // WS_C_API_EVENT_MANAGER_H
