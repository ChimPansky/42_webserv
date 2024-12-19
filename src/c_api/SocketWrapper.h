#ifndef WS_C_API_SOCKET_WRAPPER_H
#define WS_C_API_SOCKET_WRAPPER_H

#include <sys/types.h>
#include <unique_ptr.h>

#include <vector>

namespace c_api {

class SocketWrapper {
  private:
    SocketWrapper();
    SocketWrapper(const SocketWrapper&);
    SocketWrapper& operator=(const SocketWrapper&);

  public:
    SocketWrapper(int sockfd);
    ~SocketWrapper();
    int sockfd() const;

    ssize_t Recv(std::vector<char>& buf, size_t sz) const;
    ssize_t Send(const std::vector<char>& buf, size_t& start_idx, size_t sz) const;

    static std::pair<utils::unique_ptr<SocketWrapper>, utils::unique_ptr<SocketWrapper> >
    CreateSocketPair();

  private:
    int sockfd_;
};

}  // namespace c_api

#endif  // WS_C_API_SOCKET_WRAPPER_H
