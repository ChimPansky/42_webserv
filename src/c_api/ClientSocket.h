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

    ssize_t Recv(char* buf, size_t buf_sz) const;
    ssize_t Send(const std::vector<char>& buf, size_t& start_idx, size_t sz) const;
    char* sock_buf();
    size_t sock_buf_sz() const; // TODO: will be removed once we switch to RequestBuilder.RequestIsReady()
    static const size_t sock_buf_sz_ = 666;

  private:
    int sockfd_;
    char sock_buf_[sock_buf_sz_];
};

}  // namespace c_api

#endif  // WS_C_API_CLIENT_SOCKET_H
