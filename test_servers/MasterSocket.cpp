#include "MasterSocket.h"

#include <stdexcept>

namespace c_api {

// REUSEADDR in case port already open in the kernel but has no associated socket
MasterSocket::MasterSocket(in_addr_t ip, in_port_t port, bool set_nonblock)
{
    sockfd_ = ::socket(/* IPv4 */ AF_INET,
                       /* TCP */ SOCK_STREAM | (set_nonblock ? SOCK_NONBLOCK : 0),
                       /* explicit tcp */ IPPROTO_TCP);
    if (sockfd_ < 0) {
        throw std::runtime_error("cannot create socket");
    }
	int optval = 1;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = ::htons(port);
    sa.sin_addr.s_addr = ::htonl(ip);

    // bind socket to ip address and port
    if (::bind(sockfd_, (struct sockaddr*)&sa, sizeof(sa)) != 0) {
        throw std::runtime_error("cannot bind master_socket to the address");
    }

    // start listening for incoming connections, if more then SOMAXCONN are not accepted, rest will be ignored
    if (::listen(sockfd_, SOMAXCONN) != 0) {
        throw std::runtime_error("cannot bind master_socket");
    }
}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
MasterSocket::~MasterSocket() {
	/* shutdown(sockfd_, SHUT_RDWR); */
    close(sockfd_);
}

int MasterSocket::sockfd() const {
    return sockfd_;
}

}  // namespace c_api
