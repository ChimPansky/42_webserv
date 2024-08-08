#include "Socket.h"

#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // socket, bind
#include <unistd.h>      // close

namespace c_api {

Socket::Socket() {
    // with non block it is not needed to use fcntl
    _sockfd = socket(/* IPv4 */ AF_INET, /* TCP */ SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (_sockfd < 0) {
        throw "EXCEPTION PLACEHOLDER SOCK C'TOR";
    }
}

Socket::~Socket() {
    close(_sockfd);
}

void Socket::set_timeout_recv_s(int seconds) {
    timeval tv{seconds, /*us*/ 0};
    setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, (void*)&tv, sizeof(tv));
}

void Socket::set_timeout_send_s(int seconds) {
    timeval tv{seconds, /*us*/ 0};
    setsockopt(_sockfd, SOL_SOCKET, SO_SNDTIMEO, (void*)&tv, sizeof(tv));
}

int Socket::sockfd() const {
    return _sockfd;
}

}  // namespace c_api
