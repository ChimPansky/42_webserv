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
    // size_t old_sz = buf.size();
    // size_t new_sz = old_sz + sz;
    // LOG(DEBUG) << "ClientSock::Recv: buf.size(): " << old_sz << "; new_sz: " << new_sz << "; buf.capacity(): " << buf.capacity();
    // if (new_sz > buf.capacity()) {
    //     buf.resize(new_sz);
    // }

    size_t old_sz = buf.size();
    buf.resize(old_sz + sz);

    ssize_t bytes_recvd = ::recv(sockfd_, (void*)(buf.data() + old_sz), sz, MSG_NOSIGNAL);
    LOG(DEBUG) << "ClientSock::Recv after resize: buf.size(): " << buf.size() << "; buf.capacity(): " << buf.capacity();
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

char* ClientSocket::sock_buf() {
    return sock_buf_;
}

size_t ClientSocket::sock_buf_sz() const {
    return sock_buf_sz_;
}

}  // namespace c_api
