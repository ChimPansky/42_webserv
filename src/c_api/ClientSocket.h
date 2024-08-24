#ifndef WS_C_API_CLIENT_SOCKET_H
#define WS_C_API_CLIENT_SOCKET_H

#include <sys/types.h>

#include <cstddef>
#include <vector>
#include "../http/Request.h"

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

    // come up with a better signature for recv/send -->
    // Recv should take request and Send should take response as parameter
    ssize_t Recv(http::Request& rq, size_t sz = _buf_sz) const;
    ssize_t Send(const std::vector<char>& buf, size_t& start_idx, size_t sz) const;

  private:
    int _sockfd;
    static const size_t _buf_sz = 10;
    char _buf[_buf_sz];
};

}  // namespace c_api

#endif  // WS_C_API_CLIENT_SOCKET_H
