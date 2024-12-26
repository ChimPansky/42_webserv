#ifndef WS_C_API_SOCKET_WRAPPER_H
#define WS_C_API_SOCKET_WRAPPER_H

#include <maybe.h>
#include <unique_ptr.h>

#include <fstream>
#include <vector>

#define SOCK_READ_BUF_SZ 8192ul
#define SOCK_SEND_FILE_BUF_SZ 8192ul

namespace c_api {

enum SockStatus {
    RS_OK,
    RS_SOCK_CLOSED,
    RS_SOCK_ERR
};

struct RecvPackage {
    SockStatus status;
    const char* data;
    size_t data_size;
};

struct SendPackage {
    SendPackage(std::vector<char> content) : buf(content), bytes_sent(0) {}
    SendPackage(size_t buflen) : buf(), bytes_sent(0) { buf.reserve(buflen); }
    bool AllDataSent() const { return buf.size() == bytes_sent; }
    std::vector<char> buf;
    size_t bytes_sent;
};

class SendFilePackage {
  private:
    SendFilePackage(const char* path);

  public:
    static utils::unique_ptr<SendFilePackage> TryCreate(const char* path);

    bool PrepareNextChunk();

    SendPackage& chunk() { return chunk_; }
    size_t bytes_sent() { return bytes_sent_; }

    bool AllDataSent() const;

  private:
    SendPackage chunk_;
    std::ifstream ifs_;
    size_t bytes_sent_;
};

enum SockType {
    ST_TCP_V4,
};

class Socket {
  private:
    Socket();
    Socket(const Socket&);
    Socket& operator=(const Socket&);

  public:
    Socket(int sockfd);  // acquire owning of the fd
    Socket(SockType type);
    ~Socket();

    bool TrySetFlags(int flags);
    int sockfd() const { return sockfd_; }

    RecvPackage Recv(size_t max_read_sz = SOCK_READ_BUF_SZ) const;
    SockStatus Send(SendPackage&) const;
    SockStatus Send(SendFilePackage& pack) const;

    typedef std::pair<utils::unique_ptr<Socket>, utils::unique_ptr<Socket> > SocketPair;
    static utils::maybe<SocketPair> CreateLocalNonblockSocketPair();

  private:
    int sockfd_;
    char buf_[SOCK_READ_BUF_SZ];
};

}  // namespace c_api

#endif  // WS_C_API_SOCKET_WRAPPER_H
