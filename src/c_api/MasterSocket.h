#ifndef WS_C_API_MASTER_SOCKET_H
#define WS_C_API_MASTER_SOCKET_H

#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // socket, bind
#include <unistd.h>      // close

#include "Socket.h"

namespace c_api {

// socker binded to addr
class MasterSocket {
  private:
    MasterSocket();
    MasterSocket(const MasterSocket&);
    MasterSocket& operator=(const MasterSocket&);
  public:
    MasterSocket(in_addr_t ip, in_port_t port);
    ~MasterSocket();
    int sockfd() const;
  private:
    Socket _sock;
};

}  // namespace c_api

#endif  // WS_C_API_MASTER_SOCKET_H
