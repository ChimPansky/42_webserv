#ifndef WS_C_API_LISTENER_H
#define WS_C_API_LISTENER_H

#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // socket, bind
#include <unistd.h>      // close
#include <memory>      // close

namespace c_api {

// socker binded to addr
class Listener {
  private:
    Listener();
    Listener(const Listener&);
    Listener& operator=(const Listener&);
  public:
    Listener(in_addr_t ip, in_port_t port, bool set_nonblock = true);
    ~Listener();
    int sockfd() const;  // technically breaks incapsulation. mb remove
    // std::auto_ptr<int*> accept();  should it be here?
  private:
    int _sockfd;
};

}  // namespace c_api

#endif  // WS_C_API_LISTENER_H
