#ifndef WS_C_API_CLIENT_SOCKET_H
#define WS_C_API_CLIENT_SOCKET_H

#include <cstddef>
#include <vector>
#include <sys/types.h>

namespace c_api {

// socker binded to addr
class ClientSocket {
  private:
    ClientSocket();
    ClientSocket(const ClientSocket&);
    ClientSocket& operator=(const ClientSocket&);
  public:
    ClientSocket(int sockfd);
    ~ClientSocket();
    int sockfd() const;
    ssize_t  Recv(std::vector<char>& buf, size_t sz = _buf_sz) const;
    ssize_t  Send(const std::vector<char>& buf, size_t& start_idx, size_t sz) const;
  private:
    int _sockfd;
    static const size_t _buf_sz = 666;
    char _buf[_buf_sz];
};

}  // namespace c_api

#endif  // WS_C_API_CLIENT_SOCKET_H
