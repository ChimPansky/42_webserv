#ifndef WS_C_API_MASTER_SOCKET_H
#define WS_C_API_MASTER_SOCKET_H

#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // socket, bind
#include <unique_ptr.h>
#include <unistd.h>  // close

#include "ClientSocket.h"
#include "Socket.h"

namespace c_api {

// IPv4 socker binded to addr
class MasterSocket {
  private:
    MasterSocket();
    MasterSocket(const MasterSocket&);
    MasterSocket& operator=(const MasterSocket&);

  public:
    MasterSocket(in_addr_t ip, in_port_t port);
    MasterSocket(const struct sockaddr_in& addr);
    ~MasterSocket();

    inline int sockfd() const { return sock_.sockfd(); };
    const sockaddr_in& addr_in() const;

    utils::unique_ptr<ClientSocket> Accept() const;
    bool IsSameSockAddr(struct sockaddr_in&) const;

  private:
    struct sockaddr_in addr_in_;
    Socket sock_;
};

}  // namespace c_api

#endif  // WS_C_API_MASTER_SOCKET_H
