#ifndef WS_C_API_MASTER_SOCKET_H
#define WS_C_API_MASTER_SOCKET_H

#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // socket, bind
#include <unistd.h>      // close

#include "ClientSocket.h"
#include <unique_ptr.h>

namespace c_api {

// IPv4 socker binded to addr
class MasterSocket {
  private:
    MasterSocket();
    MasterSocket(const MasterSocket&);
    MasterSocket& operator=(const MasterSocket&);

  public:
    MasterSocket(in_addr_t ip, in_port_t port, bool set_nonblock = true);
    MasterSocket(const struct sockaddr_in& addr, bool set_nonblock = true);
    ~MasterSocket();
    int sockfd() const;
    const sockaddr_in& addr_in() const;
    // check result for null!
    utils::unique_ptr<ClientSocket> Accept() const;
    bool IsSameSockAddr(struct sockaddr_in&) const;

  private:
    static int socket(bool set_nonblock);

  private:
    struct sockaddr_in addr_in_;
    int sockfd_;
};

}  // namespace c_api

#endif  // WS_C_API_MASTER_SOCKET_H
