#include "ClientSocket.h"

#include <netinet/in.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

#include "utils/logger.h"

namespace c_api {

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

ssize_t ClientSocket::Recv(std::vector<char>& buf, size_t read_size) const
{
    size_t old_buf_sz = buf.size();
    buf.resize(old_buf_sz + read_size);
    ssize_t bytes_recvd = ::recv(sockfd_, (void*)(buf.data() + old_buf_sz), read_size, MSG_NOSIGNAL);
    // less bytes actually recvd than read_size -> shrink target vector
    if (bytes_recvd >= 0 && static_cast<size_t>(bytes_recvd) < read_size) {
        buf.resize(old_buf_sz + bytes_recvd);
    }
    return bytes_recvd;
}

ssize_t ClientSocket::Send(const std::vector<char>& buf, size_t& idx, size_t sz) const
{
    LOG(DEBUG) << "ClientSocket::Send";
    if (idx + sz > buf.size()) {
        throw std::runtime_error("idx is too big");
    }
    ssize_t bytes_sendd = ::send(sockfd_, buf.data() + idx, sz, MSG_NOSIGNAL);
    if (bytes_sendd > 0) {
        idx += bytes_sendd;
    }
    return bytes_sendd;
}

}  // namespace c_api
