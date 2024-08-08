#include "MasterSocket.h"

namespace c_api {

// REUSEADDR in case port already open in cernel but has no associated socket
MasterSocket::MasterSocket(in_addr_t ip, in_port_t port) {
	int optval = 1;
    setsockopt(_sock.sockfd(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(ip);
    // bind to ip address and port
    if (::bind(_sock.sockfd(), (struct sockaddr*)&sa, sizeof(sa)) != 0) {
        throw "EXCEPTION PLACEHOLDER SOCK BIND";
    }
}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
MasterSocket::~MasterSocket() {
	/* shutdown(_sock.sockfd(), SHUT_RDWR); */
}

int MasterSocket::sockfd() const {
    return _sock.sockfd();
}

}  // namespace c_api
