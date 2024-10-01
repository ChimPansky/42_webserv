#include <set>
#include <string>
#include <algorithm>

#include "common.h"
#include <errno.h>
#include <iostream>

int main() {
    std::string ip = "127.0.0.1";
    unsigned short port = 12346;
    int sockfd = CreateAndBindSocket(INADDR_LOOPBACK, port);
    std::set<int> SlaveSockets;

    typedef std::set<int>::iterator set_it;

    while (1) {
        fd_set select_rd_set;
        FD_ZERO(&select_rd_set);
        FD_SET(sockfd, &select_rd_set);
        for (set_it it = SlaveSockets.begin();
             it != SlaveSockets.end();
             ++it) {
            FD_SET(*it, &select_rd_set);
        }
        int maxfd = SlaveSockets.empty() ? sockfd : std::max(sockfd, *SlaveSockets.rbegin());

        /* on select wait for ready fds or timeout */
        int num_of_fds = select(maxfd + 1, &select_rd_set, /* write fds */ NULL, /* err fds */ NULL, /* timeout */ NULL);
        if (num_of_fds <= 0) {
            // select errors or empty set
            std::cout << " no connections :(" << std::endl;
            continue;
        }
        for (int i = 0; i <= maxfd; ++i) {
            set_it it;
            if (FD_ISSET(i, &select_rd_set) && ((it = SlaveSockets.find(i)) != SlaveSockets.end())) {
                static char buf[1024];
                int recv_sz = recv(*it, buf, 1023, MSG_NOSIGNAL);
                if (recv_sz <= 0 && errno != EAGAIN) {
                    close(*it);
                    SlaveSockets.erase(it);
                    std::cout << "connection closed" << std::endl;
                } else if (recv_sz != 0) {
                    buf[recv_sz] = 0;
                    std::cout << buf;
                    if (buf[0] == '\0') {
                        std::cout << std::endl;
                    }
                    send(*it, buf, recv_sz, MSG_NOSIGNAL);
                    ++it;
                }
            }
        }
        if (FD_ISSET(sockfd, &select_rd_set)) {
            int slave = accept(sockfd, 0, 0);
            SlaveSockets.insert(slave);
            std::cout << " new connection !" << std::endl;
        }
    }
    close(sockfd);
}
