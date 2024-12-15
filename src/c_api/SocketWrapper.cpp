#include "SocketWrapper.h"

#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>

namespace c_api {

SocketWrapper::SocketWrapper(int fd) : sockfd_(fd) {}

SocketWrapper::~SocketWrapper() {
    close(sockfd_);
}

int SocketWrapper::sockfd() const { return sockfd_; }

ssize_t SocketWrapper::Recv(std::vector<char>& buf, size_t read_size) const
{
    return ::recv(sockfd_, (void*)(buf.data() + buf.size() - read_size), read_size, MSG_NOSIGNAL);
}

ssize_t SocketWrapper::Send(const std::vector<char>& buf, size_t& idx, size_t sz) const
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

}  // namespace c_api
