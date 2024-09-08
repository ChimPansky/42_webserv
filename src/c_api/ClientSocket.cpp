#include "ClientSocket.h"

#include <netinet/in.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>
#include "utils/logger.h"

namespace c_api {

const size_t ClientSocket::sock_buf_sz_;

ClientSocket::ClientSocket(int fd) : sockfd_(fd)
{}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
ClientSocket::~ClientSocket()
{
    /* shutdown(sockfd_, SHUT_RDWR); */
    close(sockfd_);
}

int ClientSocket::sockfd() const
{
    return sockfd_;
}

ssize_t ClientSocket::Recv(std::vector<char>& buf, size_t sz) const
{
    ssize_t bytes_recvd = ::recv(sockfd_, (void*)sock_buf_, std::min(sz, sock_buf_sz_), MSG_NOSIGNAL);
    if (bytes_recvd > 0) {
        size_t init_sz = buf.size();
        buf.resize(init_sz + bytes_recvd);
        std::memcpy(buf.data() + init_sz, sock_buf_, bytes_recvd);
    }
    return bytes_recvd;
}

ssize_t ClientSocket::Send(const std::vector<char>& buf, size_t& idx, size_t sz) const
{
    // TODO: send in chunks (like recv)
    LOG(DEBUG) << "Sending " << sz << " bytes";
    LOG(DEBUG) << "idx: " << idx;
    LOG(DEBUG) << "buf.data(): " << buf.data();

    if (idx + sz > buf.size()) {
        throw std::runtime_error("idx is too big");
    }
    ssize_t bytes_sendd = ::send(sockfd_, buf.data() + idx, sz, MSG_NOSIGNAL);
    if (bytes_sendd > 0) {
        idx += bytes_sendd;
    }
    return bytes_sendd;
}

size_t ClientSocket::sock_buf_sz() const {
    return sock_buf_sz_;
}

}  // namespace c_api
