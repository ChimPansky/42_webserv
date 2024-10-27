#include "ReadSocket.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

#include <logger.h>

namespace c_api {

ReadSocket::ReadSocket(int fd) : sockfd_(fd)
{}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
ReadSocket::~ReadSocket()
{
    /* shutdown(sockfd_, SHUT_RDWR); */
    close(sockfd_);
}

int ReadSocket::sockfd() const
{
    return sockfd_;
}

ssize_t ReadSocket::Recv()
{
    return ::recv(sockfd_, buf_, RECV_BUF_SZ, MSG_NOSIGNAL);
}

const char* ReadSocket::buf() const {
    return buf_;
}

}  // namespace c_api
