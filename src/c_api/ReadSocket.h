#ifndef WS_C_API_READ_SOCKET_H
#define WS_C_API_READ_SOCKET_H

#include <sys/types.h>

#define RECV_BUF_SZ 100000

namespace c_api {

// socker binded to addr
// consider add sockaddr_t to check for incoming ip?
class ReadSocket {
  private:
    ReadSocket();
    ReadSocket(const ReadSocket&);
    ReadSocket& operator=(const ReadSocket&);

  public:
    ReadSocket(int sockfd);
    ~ReadSocket();
    int sockfd() const;

    // come up with a better signature for recv/send
    ssize_t Recv();
    const char* buf() const;

  private:
    int sockfd_;
    char buf_[RECV_BUF_SZ];
};

}  // namespace c_api

#endif  // WS_C_API_READ_SOCKET_H
