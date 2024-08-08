#ifndef WS_C_API_SOCKET_H
#define WS_C_API_SOCKET_H

namespace c_api {

// tcp socket for IPv4
class Socket {
  private:
    Socket(const Socket&);
    Socket& operator=(const Socket&);
  public:
    Socket();
    ~Socket();
    void set_timeout_recv_s(int seconds);
    void set_timeout_send_s(int seconds);
    int sockfd() const;
  private:
    int _sockfd;
};

}  // namespace c_api

#endif  // WS_C_API_SOCKET_H
