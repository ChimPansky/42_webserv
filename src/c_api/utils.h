#ifndef WS_C_API_UTILS_H
#define WS_C_API_UTILS_H

#include <netinet/in.h>

#include <string>

namespace c_api {

in_addr_t IPv4FromString(const std::string& ip_str);
std::string IPv4ToString(in_addr_t ip);
struct sockaddr_in GetIPv4SockAddr(in_addr_t ip, in_port_t port);

}  // namespace c_api

#endif
