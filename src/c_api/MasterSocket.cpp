#include "MasterSocket.h"

#include <cstring>
#include <stdexcept>

#include "c_api_utils.h"


namespace {
int CreateTcpSocket(bool set_nonblock)
{
    int sockfd = ::socket(/* IPv4 */ AF_INET,
                          /* TCP */ SOCK_STREAM | (set_nonblock ? SOCK_NONBLOCK : 0),
                          /* explicit tcp */ IPPROTO_TCP);
    if (sockfd < 0) {
        throw std::runtime_error("cannot create socket");
    }
    int optval = 1;
    ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    return sockfd;
}

// TODO convertion from addrin to addr here is technically a UB
void BindAndListen(int sockfd_, struct sockaddr_in& addr_in)
{
    // bind socket to ip address and port
    if (::bind(sockfd_, (struct sockaddr*)&addr_in, sizeof(addr_in)) != 0) {
        throw std::runtime_error("cannot bind master_socket to the address");
    }

    // start listening for incoming connections, if more then SOMAXCONN are not accepted,
    // rest will be ignored
    if (::listen(sockfd_, SOMAXCONN) != 0) {
        throw std::runtime_error("cannot bind master_socket");
    }
}
}  // namespace

namespace c_api {

// REUSEADDR in case port already open in the kernel but has no associated socket
MasterSocket::MasterSocket(in_addr_t ip, in_port_t port, bool set_nonblock)
    : sock_(CreateTcpSocket(set_nonblock))
{
    addr_in_ = GetIPv4SockAddr(ip, port);
    BindAndListen(sockfd(), addr_in_);
}

MasterSocket::MasterSocket(const struct sockaddr_in& addr, bool set_nonblock)
    : addr_in_(addr), sock_(CreateTcpSocket(set_nonblock))
{
    BindAndListen(sockfd(), addr_in_);
}

utils::unique_ptr<ClientSocket> MasterSocket::Accept() const
{
    struct sockaddr_in addr = {};
    socklen_t addr_len = sizeof(addr);
    int client_fd = ::accept(sockfd(), (struct sockaddr*)&addr, &addr_len);
    if (client_fd < 0) {
        return utils::unique_ptr<ClientSocket>();
    }
    return utils::unique_ptr<ClientSocket>(new ClientSocket(client_fd, addr));
}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
MasterSocket::~MasterSocket()
{
    /* shutdown(sockfd_, SHUT_RDWR); */
}

const sockaddr_in& MasterSocket::addr_in() const
{
    return addr_in_;
}


bool MasterSocket::IsSameSockAddr(struct sockaddr_in& addr) const
{
    return (memcmp(&addr, &addr_in_, sizeof(addr_in_)) == 0);
}

}  // namespace c_api
