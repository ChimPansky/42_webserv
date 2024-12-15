#ifndef WS_C_API_CLIENT_SOCKET_H
#define WS_C_API_CLIENT_SOCKET_H

#include <netinet/in.h>
#include <sys/types.h>

#include <cstddef>
#include <vector>

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
    int sockfd() const;
    const sockaddr_in& addr_in() const;

    // come up with a better signature for recv/send
    ssize_t Recv(std::vector<char>& buf, size_t sz) const;
    ssize_t Send(const std::vector<char>& buf, size_t& start_idx, size_t sz) const;
    size_t buf_sz() const;  // TODO remove

  private:
    int sockfd_;
    sockaddr_in addr_in_;
};

}  // namespace c_api

#endif  // WS_C_API_CLIENT_SOCKET_H
