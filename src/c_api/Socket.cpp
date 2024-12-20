#include "Socket.h"

#include <netinet/in.h>
#include <unistd.h>

#include <cstdio>
#include <stdexcept>

namespace c_api {

Socket::Socket(int fd) : sockfd_(fd)
{}

Socket::~Socket()
{
    close(sockfd_);
}

int Socket::sockfd() const
{
    return sockfd_;
}

RecvPackage Socket::Recv(size_t read_size) const
{
    RecvPackage pack = {};
    pack.data_size =
        ::recv(sockfd_, (void*)buf_, std::min(read_size, SOCK_READ_BUF_SZ), MSG_NOSIGNAL);
    if (pack.data_size < 0) {
        pack.status = RS_SOCK_ERR;
    } else if (pack.data_size == 0) {
        pack.status = RS_SOCK_CLOSED;
    } else {
        pack.status = RS_OK;
        pack.data = buf_;
    }
    return pack;
}

SockStatus Socket::Send(SendPackage& pack) const
{
    ssize_t bytes_sent = ::send(sockfd_, pack.buf.data() + pack.bytes_sent,
                                pack.buf.size() - pack.bytes_sent, MSG_NOSIGNAL);
    if (bytes_sent > 0) {
        pack.bytes_sent += bytes_sent;
        return RS_OK;
    }
    return RS_SOCK_ERR;
}

std::pair<utils::unique_ptr<Socket>, utils::unique_ptr<Socket> > Socket::CreateUnixSocketPair(
    bool set_nonblock)
{
    int socket_fds[2];

    if (::socketpair(AF_UNIX, SOCK_STREAM | (set_nonblock ? SOCK_NONBLOCK : 0),
                     /*default for unix sock*/ 0, socket_fds) < 0) {
        std::perror("socketpair failed");
        return std::make_pair(utils::unique_ptr<Socket>(NULL), utils::unique_ptr<Socket>(NULL));
    }
    return std::make_pair(utils::unique_ptr<Socket>(new Socket(socket_fds[0])),
                          utils::unique_ptr<Socket>(new Socket((socket_fds[1]))));
}

}  // namespace c_api
