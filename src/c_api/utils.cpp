#include "utils.h"

#include <string>
#include <sstream>
#include <netinet/in.h>

namespace c_api {

in_addr_t ipv4_from_string(const std::string& ip_str) {
    if (ip_str == "localhost") {
        return INADDR_LOOPBACK;
    }

    std::stringstream iss(ip_str);

    in_addr_t ipv4 = 0;
    for(int i = 0; i < 4; ++i) {
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

std::string ipv4_to_string(in_addr_t ipv4) {
    std::stringstream ss;
    ss << (ipv4 >> 24)
       << '.' << ((ipv4 >> 16) % 256)
       << '.' << ((ipv4 >> 8) % 256)
       << '.' << (ipv4 % 256);
    return ss.str();
}

}  // namespace c_api

/*
#include <iostream>
int main() {
    std::string ip;
    ip = "127.0.0.1";
    std::cout << ip << " == " << c_api::ipv4_to_string(INADDR_LOOPBACK) << std::endl;
    std::cout << INADDR_LOOPBACK << " == " << c_api::ipv4_from_string(ip) << std::endl;
    ip = "0.0.0.0";
    std::cout << ip << " == " << c_api::ipv4_to_string(INADDR_ANY) << std::endl;
    std::cout << INADDR_ANY << " == " << c_api::ipv4_from_string(ip) << std::endl;
}
*/
