#include <errno.h>

#include <iostream>

#include "common.h"

int main()
{
    unsigned short port = 8080;
    int sockfd = CreateAndBindSocket(IPv4FromString("127.0.0.1"), port);

    std::cout << "listening on localhost:" << port << "..." << std::endl;
    struct sockaddr addr;
    socklen_t addr_len;
    int slave_socket_fd = -1;

    while (true) {
        slave_socket_fd = accept(sockfd, &addr, &addr_len);
        if (slave_socket_fd == -1 && errno != EAGAIN) {
            std::cout << "unexpexcted error" << std::endl;
            break;
        } else if (slave_socket_fd == -1 && errno == EAGAIN) {
            usleep(1000 * 1000);
            continue;
        }
        std::cout << "opened new connection" << std::endl;
        char buf[128];
        errno = 0;
        while (long a = recv(slave_socket_fd, buf, 127, MSG_NOSIGNAL)) {
            if (errno == EAGAIN) {
                continue;
            }
            buf[a] = '\0';
            std::cout << buf;
            if (buf[0] == '\0') {
                std::cout << std::endl;
            }
            long b = 0;
            while (b != a) {
                b += send(slave_socket_fd, buf + b, a, MSG_NOSIGNAL);
            }
        }
        close(slave_socket_fd);  // we need select to keep alive
        std::cout << "connection closed" << std::endl;
    }
    close(sockfd);
}
