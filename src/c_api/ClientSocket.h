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
    const char* buf() const;
    size_t buf_sz();

    // come up with a better signature for recv/send -->
    // --> removed parameters for Recv
    // --> always try to read clientsocket._buf_sz bytes into clientsocket._buf;
    ssize_t Recv() const;
    ssize_t Send(const std::vector<char>& buf, size_t& start_idx, size_t sz) const;

  private:
    int _sockfd;
    static const size_t _buf_sz = 10;   // keeping this small while working on http-parsing
    char _buf[_buf_sz];
};

}  // namespace c_api

#endif  // WS_C_API_CLIENT_SOCKET_H
