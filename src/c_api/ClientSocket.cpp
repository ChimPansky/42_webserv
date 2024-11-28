#include "ClientSocket.h"

#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <vector>

#include <logger.h>

namespace c_api {

ClientSocket::ClientSocket(int fd, sockaddr_in addr_in) : sockfd_(fd), addr_in_(addr_in)
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

const sockaddr_in& ClientSocket::addr_in() const {
    return addr_in_;
}

ssize_t ClientSocket::Recv(std::vector<char>& buf, size_t read_size) const
{
    ssize_t bytes_read = ::recv(sockfd_, (void*)(buf.data()), read_size, MSG_NOSIGNAL);
    if (bytes_read < 0) {
        buf.resize(0);
    } else if (static_cast<size_t>(bytes_read) < read_size) {
        buf.resize(bytes_read);
    }
    return bytes_read;
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
