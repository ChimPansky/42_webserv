#include <netinet/in.h>

#include <sstream>
#include <string>

namespace c_api {

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

std::string IPv4ToString(in_addr_t ipv4)
{
    std::stringstream ss;
    ss << (ipv4 >> 24)                 //
       << '.' << ((ipv4 >> 16) % 256)  //
       << '.' << ((ipv4 >> 8) % 256)   //
       << '.' << (ipv4 % 256);
    return ss.str();
}

struct sockaddr_in GetIPv4SockAddr(in_addr_t ip, in_port_t port)
{
    struct sockaddr_in addr_in = {};
    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = ::htonl(ip);
    addr_in.sin_port = ::htons(port);
    return addr_in;
}

std::string PrintIPv4SockAddr(const sockaddr_in& addr_in)
{
    std::stringstream ss;
    if (addr_in.sin_family != AF_INET) {
        ss << "Not an IPv4 addr";
    } else {
        ss << IPv4ToString(htonl(addr_in.sin_addr.s_addr)) << ":" << htons(addr_in.sin_port);
    }
    return ss.str();
}

}  // namespace c_api

/*
#include <iostream>
int main() {
    std::string ip;
    ip = "127.0.0.1";
    std::cout << ip << " == " << c_api::IPv4ToString(INADDR_LOOPBACK) << std::endl;
    std::cout << INADDR_LOOPBACK << " == " << c_api::IPv4FromString(ip) << std::endl;
    ip = "0.0.0.0";
    std::cout << ip << " == " << c_api::IPv4ToString(INADDR_ANY) << std::endl;
    std::cout << INADDR_ANY << " == " << c_api::IPv4FromString(ip) << std::endl;
}
*/
