#include <cstring>
#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // socket, bind
#include <unistd.h>      // close
#include <string>      // close
#include <stdexcept>      // close
#include <sstream>      // close

in_addr_t IPv4FromString(const std::string& ip_str)
{
    if (ip_str == "localhost") {
        return INADDR_LOOPBACK;
    }

    std::stringstream iss(ip_str);

    in_addr_t ipv4 = 0;
    for (int i = 0; i < 4; ++i) {
        in_addr_t part;
        iss >> part;
        if (iss.fail() || part > 255) {
            throw std::runtime_error("Invalid IP address - Expected [0, 255]");
        }
        ipv4 |= part << (8 * (3 - i));
        if (i != 3) {
            char delim;
            iss >> delim;
            if (iss.fail() || delim != '.') {
                throw std::runtime_error("Invalid IP address - Expected '.' delimiter");
            }
        }
    }
    if (!iss.eof()) {
        throw std::runtime_error("Invalid IP address - expected [0-255].[0-255].[0-255].[0-255]");
    }
    return ipv4;
}


int CreateAndBindSocket(in_addr_t ip, in_port_t port, bool set_nonblock = true)
{
    int sockfd_ = ::socket(/* IPv4 */ AF_INET,
                            /* TCP */ SOCK_STREAM | (set_nonblock ? SOCK_NONBLOCK : 0),
                            /* explicit tcp */ IPPROTO_TCP);
    if (sockfd_ < 0) {
        throw std::runtime_error("cannot create socket");
    }
	int optval = 1;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in sa_in = {};
    socklen_t socklen_ = sizeof(sa_in);
    sa_in.sin_family = AF_INET;
    sa_in.sin_port = ::htons(port);
    sa_in.sin_addr.s_addr = ::htonl(ip);

    struct sockaddr sockaddr = {};

    memcpy(&sockaddr, &sa_in, socklen_);

    // bind socket to ip address and port
    if (::bind(sockfd_, &sockaddr, socklen_) != 0) {
        throw std::runtime_error("cannot bind master_socket to the address");
    }

    // start listening for incoming connections, if more then SOMAXCONN are not accepted, rest will be ignored
    if (::listen(sockfd_, SOMAXCONN) != 0) {
        throw std::runtime_error("cannot bind master_socket");
    }
    return sockfd_;
}
