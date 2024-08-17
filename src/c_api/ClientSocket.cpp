#include "ClientSocket.h"

#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>

#include <vector>
#include <stdexcept>

namespace c_api {

const size_t ClientSocket::_buf_sz;

ClientSocket::ClientSocket(int fd)
  : _sockfd(fd)
{}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
ClientSocket::~ClientSocket() {
	/* shutdown(_sockfd, SHUT_RDWR); */
    close(_sockfd);
}

int ClientSocket::sockfd() const {
    return _sockfd;
}

ssize_t  ClientSocket::Recv(std::vector<char>& buf, size_t sz) const {
    ssize_t bytes_recvd = ::recv(_sockfd, (void*)_buf, std::min(sz, _buf_sz), MSG_NOSIGNAL);
    if (bytes_recvd > 0) {
        size_t init_sz = buf.size();
        buf.resize(init_sz + bytes_recvd);
        std::memcpy(buf.data() + init_sz, _buf, bytes_recvd);
    }
    return bytes_recvd;
}

ssize_t  ClientSocket::Send(const std::vector<char>& buf, size_t& idx, size_t sz) const {
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
