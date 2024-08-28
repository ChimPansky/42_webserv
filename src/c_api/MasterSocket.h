#ifndef WS_C_API_MASTER_SOCKET_H
#define WS_C_API_MASTER_SOCKET_H

#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // socket, bind
#include <unistd.h>      // close

#include "ClientSocket.h"
#include "utils/unique_ptr.h"

namespace c_api {

// socker binded to addr
class MasterSocket {
  private:
    MasterSocket();
    MasterSocket(const MasterSocket&);
    MasterSocket& operator=(const MasterSocket&);

  public:
    MasterSocket(in_addr_t ip, in_port_t port, bool set_nonblock = true);
    ~MasterSocket();
    int sockfd() const;
    // check result for null!
    utils::unique_ptr<ClientSocket> Accept() const;

  private:
    int sockfd_;
};

}  // namespace c_api

#endif  // WS_C_API_MASTER_SOCKET_H
