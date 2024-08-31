#ifndef WS_C_API_CLIENT_SOCKET_H
#define WS_C_API_CLIENT_SOCKET_H

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
    ClientSocket(int sockfd);
    ~ClientSocket();
    int sockfd() const;

    // come up with a better signature for recv/send
    ssize_t Recv(std::vector<char>& buf, size_t sz = buf_sz_) const;
    ssize_t Send(const std::vector<char>& buf, size_t& start_idx, size_t sz) const;

  private:
    int sockfd_;
    static const size_t buf_sz_ = 10;
    char buf_[buf_sz_];
};

}  // namespace c_api

#endif  // WS_C_API_CLIENT_SOCKET_H
