#ifndef WS_C_API_CLIENT_SOCKET_H
#define WS_C_API_CLIENT_SOCKET_H

#include <netinet/in.h>
#include <sys/types.h>

#include <cstddef>

#include "Socket.h"

namespace c_api {

// socker binded to addr
// consider add sockaddr_t to check for incoming ip?
class ClientSocket {
  private:
    ClientSocket();
    ClientSocket(const ClientSocket&);
    ClientSocket& operator=(const ClientSocket&);

  public:
    ClientSocket(int sockfd, sockaddr_in addr_in);
    ~ClientSocket();
    inline int sockfd() const { return sock_.sockfd(); };
    const sockaddr_in& addr_in() const { return addr_in_; };
    RecvPackage Recv(size_t read_size = SOCK_READ_BUF_SZ) const { return sock_.Recv(read_size); }
    SockStatus Send(SendPackage& pack) const { return sock_.Send(pack); };

  private:
    Socket sock_;
    sockaddr_in addr_in_;
};

}  // namespace c_api

#endif  // WS_C_API_CLIENT_SOCKET_H
