#include "ClientSocket.h"

#include <netinet/in.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace c_api {

const size_t ClientSocket::_kBufSize;

ClientSocket::ClientSocket(int fd) : _sockfd(fd)
{}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
ClientSocket::~ClientSocket()
{
    /* shutdown(_sockfd, SHUT_RDWR); */
    close(_sockfd);
}

int ClientSocket::sockfd() const
{
    return _sockfd;
}

const char* ClientSocket::buf() const {
    return _buf;
}

ssize_t ClientSocket::Recv()
{
    ssize_t bytes_recvd = ::recv(_sockfd, (void*)_buf, _kBufSize, MSG_NOSIGNAL);
    return bytes_recvd;
}

ssize_t ClientSocket::Send(const std::vector<char>& buf, size_t& idx, size_t sz) const
{
    if (idx + sz > buf.size()) {
        throw std::runtime_error("idx is too big");
    }
    ssize_t bytes_sendd = ::send(_sockfd, buf.data() + idx, sz, MSG_NOSIGNAL);
    if (bytes_sendd > 0) {
        idx += bytes_sendd;
    }
    return bytes_sendd;
}

}  // namespace c_api
