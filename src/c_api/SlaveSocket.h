#ifndef WS_C_API_SLAVE_SOCKET_H
#define WS_C_API_SLAVE_SOCKET_H

namespace c_api {

// socker binded to addr
class SlaveSocket {
  private:
    SlaveSocket();
    SlaveSocket(const SlaveSocket&);
    SlaveSocket& operator=(const SlaveSocket&);
  public:
    SlaveSocket(int sockfd);
    ~SlaveSocket();
    int sockfd() const;
    int send() const; 
    int recv() const;
  private:
    int _sockfd;
    static const int _buf_sz = 666;
    char _buf[_buf_sz];
};

}  // namespace c_api

#endif  // WS_C_API_SLAVE_SOCKET_H
