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

ssize_t Socket::Recv(std::vector<char>& buf, size_t read_size) const
{
    return ::recv(sockfd_, (void*)(buf.data() + buf.size() - read_size), read_size, MSG_NOSIGNAL);
}

ssize_t Socket::Send(const std::vector<char>& buf, size_t& idx, size_t sz) const
{
    if (idx + sz > buf.size()) {
        throw std::runtime_error("index is too big");
    }
    ssize_t bytes_sent = send(sockfd_, buf.data() + idx, sz, MSG_NOSIGNAL);
    if (bytes_sent > 0) {
        idx += bytes_sent;
    }
    return bytes_sent;
}

std::pair<utils::unique_ptr<Socket>, utils::unique_ptr<Socket> > Socket::CreateSocketPair()
{
    int socket_fds[2];

    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, socket_fds) < 0) {
        std::perror("socketpair failed");
        return std::make_pair(utils::unique_ptr<Socket>(NULL), utils::unique_ptr<Socket>(NULL));
    }
    return std::make_pair(utils::unique_ptr<Socket>(new Socket(socket_fds[0])),
                          utils::unique_ptr<Socket>(new Socket((socket_fds[1]))));
}

}  // namespace c_api
