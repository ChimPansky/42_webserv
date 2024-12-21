#include "Socket.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>

namespace c_api {

Socket::Socket(int fd) : sockfd_(fd)
{}

Socket::Socket(SockType type)
{
    if (type != ST_TCP_V4) {
        throw std::logic_error("Sock constructor is only implemented for TCP sockets");
    }
    sockfd_ = ::socket(/* IPv4 */ AF_INET,
                       /* TCP */ SOCK_STREAM,
                       /* explicit tcp */ IPPROTO_TCP);
    if (sockfd_ < 0) {
        throw std::runtime_error("cannot create socket");
    }
}

bool Socket::TrySetFlags(int flags)
{
    int cur_flags = fcntl(sockfd_, F_GETFL);
    if (cur_flags < 0) {
        return false;
    }
    int flags_to_set = (cur_flags ^ flags) & flags;
    if (!flags_to_set) {
        return true;
    }
    return (fcntl(sockfd_, F_SETFL, cur_flags | flags_to_set) >= 0);
}

Socket::~Socket()
{
    close(sockfd_);
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

utils::maybe<Socket::SocketPair> Socket::CreateLocalNonblockSocketPair()
{
    int socket_fds[2];

    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK,
                     /*default for unix sock*/ 0, socket_fds) < 0) {
        return utils::maybe<SocketPair>();
    }
    return std::make_pair(utils::unique_ptr<Socket>(new Socket(socket_fds[0])),
                          utils::unique_ptr<Socket>(new Socket((socket_fds[1]))));
}

}  // namespace c_api
