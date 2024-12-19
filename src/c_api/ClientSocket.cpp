#include "ClientSocket.h"

#include <logger.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <vector>

namespace c_api {

ClientSocket::ClientSocket(int fd, sockaddr_in addr_in) : sock_(fd), addr_in_(addr_in)
{}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
ClientSocket::~ClientSocket()
{
    /* shutdown(sockfd_, SHUT_RDWR); */
}

const sockaddr_in& ClientSocket::addr_in() const
{
    return addr_in_;
}

ssize_t ClientSocket::Recv(std::vector<char>& buf, size_t read_size) const
{
    return sock_.Recv(buf, read_size);
}

ssize_t ClientSocket::Send(const std::vector<char>& buf, size_t& idx, size_t sz) const
{
    return sock_.Send(buf, idx, sz);
}

}  // namespace c_api
