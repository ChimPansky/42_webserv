#ifndef WS_C_API_UTILS_H
#define WS_C_API_UTILS_H

#include <string>
#include <netinet/in.h>

namespace c_api {

in_addr_t IPv4FromString(const std::string& ip_str);
std::string ipv4_to_string(in_addr_t ip);

}  // namespace c_api

#endif  // WS_C_API_UTILS_H
