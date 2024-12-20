#ifndef WS_C_API_SOCKET_WRAPPER_H
#define WS_C_API_SOCKET_WRAPPER_H

#include <sys/types.h>
#include <unique_ptr.h>

#include <vector>

namespace c_api {

class Socket {
  private:
    Socket();
    Socket(const Socket&);
    Socket& operator=(const Socket&);

  public:
    Socket(int sockfd);
    ~Socket();
    int sockfd() const;

    ssize_t Recv(std::vector<char>& buf, size_t sz) const;
    ssize_t Send(const std::vector<char>& buf, size_t& start_idx, size_t sz) const;

    static std::pair<utils::unique_ptr<Socket>, utils::unique_ptr<Socket> > CreateSocketPair();

  private:
    int sockfd_;
};

}  // namespace c_api

#endif  // WS_C_API_SOCKET_WRAPPER_H
