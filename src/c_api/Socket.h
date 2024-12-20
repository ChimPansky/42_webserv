#ifndef WS_C_API_SOCKET_WRAPPER_H
#define WS_C_API_SOCKET_WRAPPER_H

#include <sys/types.h>
#include <unique_ptr.h>

#include <vector>

#define SOCK_READ_BUF_SZ 512ul

namespace c_api {

enum SockStatus {
    RS_OK,
    RS_SOCK_CLOSED,
    RS_SOCK_ERR
};

struct RecvPackage {
    SockStatus status;
    const char* data;
    ssize_t data_size;
};

// TODO: imagine copying rs body 3 times till it reaches this point
//  probably send headers and body separately, and buf is unique ptr to vector
struct SendPackage {
    SendPackage(std::vector<char> content) : buf(content), bytes_sent(0) {}
    bool AllDataSent() const { return buf.size() == bytes_sent; }
    std::vector<char> buf;
    size_t bytes_sent;
};

class Socket {
  private:
    Socket();
    Socket(const Socket&);
    Socket& operator=(const Socket&);

  public:
    Socket(int sockfd);
    ~Socket();
    int sockfd() const;

    RecvPackage Recv(size_t max_read_sz = SOCK_READ_BUF_SZ) const;
    SockStatus Send(SendPackage&) const;

    static std::pair<utils::unique_ptr<Socket>, utils::unique_ptr<Socket> > CreateUnixSocketPair(
        bool set_nonblock = true);

  private:
    int sockfd_;
    char buf_[SOCK_READ_BUF_SZ];
};

}  // namespace c_api

#endif  // WS_C_API_SOCKET_WRAPPER_H
