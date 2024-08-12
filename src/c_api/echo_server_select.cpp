#include <set>
#include <string>
#include <algorithm>

#include "utils.h"
#include "MasterSocket.h"

namespace c_api {

int main() {
    std::string ip = "127.0.0.1";
    unsigned short port = 12346;
    c_api::MasterSocket master(c_api::ipv4_from_string(ip), port);
    std::set<int> SlaveSockets;


    typedef std::set<int>::const_iterator set_it;

    while (1) {
        fd_set select_rd_set;
        FD_ZERO(&select_rd_set);
        FD_SET(master.sockfd(), &select_rd_set);
        for (set_it it = SlaveSockets.begin();
             it != SlaveSockets.end();
             ++it)
        {
            FD_SET(*it, &select_rd_set);
        }
        int maxfd = SlaveSockets.empty() ? master.sockfd() : std::max(master.sockfd(), *SlaveSockets.rbegin());

        /* on select wait for ready fds or timeout */
        int num_of_fds = select(maxfd, &select_rd_set, /* write fds */ NULL, /* err fds */ NULL, /* timeout */ NULL);
        if (num_of_fds <= 0) {
            // select errors or empty set
            continue;
        }
        for (set_it it = SlaveSockets.begin();
             it != SlaveSockets.end(); /* increment in tht body */)
        {
            if (FD_ISSET(*it, &select_rd_set)) {
                static char buf[1024];
                int recv_sz = recv(*it, buf, 1024, MSG_NOSIGNAL);
                if (recv_sz == 0 && errno != EAGAIN) {
                    close(*it);
                    it = SlaveSockets.erase(it);
                } else if (recv_sz != 0) {
                    send(*it, buf, recv_sz, MSG_NOSIGNAL);
                    ++it;
                }
            }
        }
        if (FD_ISSET(master.sockfd(), &select_rd_set)) {
            int slave = accept(master.sockfd(), 0, 0);
            SlaveSockets.insert(slave);
        }
    }

}

}  // namespace c_api
