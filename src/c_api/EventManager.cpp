#include "EventManager.h"

#include <netinet/in.h>  // select
#include <sys/epoll.h>  // for epoll

#include <stdexcept>
#include "../utils/logger.h"

namespace c_api {

utils::unique_ptr<EventManager> EventManager::instance_(NULL);

// private c'tor
EventManager::EventManager(EventManager::MultiplexType mx_type) : mx_type_(mx_type)
{}

void EventManager::init(EventManager::MultiplexType mx_type)
{
    if (EventManager::instance_) {
        throw std::runtime_error("event manager was already initialized");
    }
    EventManager::instance_.reset(new EventManager(mx_type));
}

EventManager& EventManager::get()
{
    if (!EventManager::instance_) {
        throw std::runtime_error("Event manager is not initialised");
    }
    return (*EventManager::instance_);
}

int EventManager::CheckOnce()
{
    switch (mx_type_) {
        case MT_SELECT:
            return CheckWithSelect_();
        case MT_POLL:
            return CheckWithPoll_();
        case MT_EPOLL:
            return CheckWithEpoll_();
    }
    return 1;
}

int EventManager::CheckWithPoll_() {
    LOG(ERROR) << "CheckWithPoll not implemented";
    return 1;
}

int EventManager::CheckWithEpoll_() {
    LOG(DEBUG) << "CheckWithEpoll";
    struct epoll_event ev;
    struct epoll_event events[EPOLL_MAX_EVENTS];
    int epoll_fd = epoll_create(1); // argument just for backwards compatibility --> its ignored since Linux 2.6.8, but must be > 0
    if (epoll_fd == -1) {
        LOG(ERROR) << "epoll_create failed";
        return 1;
    }
    ev.events = EPOLLIN;
    ev.data.fd = 0;
    (void)events;
    return 0;

// -----------------------------------------------
//     // poll (epoll_create,
// // epoll_ctl, epoll_wait),

// #include "utils.h"
// #include "sys/epoll.h"
// #include <cstddef>
// #include <cstring>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <iostream>
// #include <fcntl.h>
// #include <unistd.h>
// #include <cstdio>

// #define MAX_EVENTS 10


// std::ostream& operator<<(std::ostream& os, const sockaddr_in& addr) {
//     os << "sin_family: " << addr.sin_family << std::endl;
//     os << "sin_port: " << addr.sin_port << std::endl;
//     os << "sin_addr: " << addr.sin_addr.s_addr << std::endl;
//     return os;
// }

// int main() {
//     const char* ip1 = "127.0.0.1";
//     in_port_t port1 = 5001;



//     struct epoll_event ev; // master event
//     struct epoll_event events[MAX_EVENTS]; // client events...
//     int epollfd = epoll_create(1); // argument is ignored since Linux 2.6.8, but must be > 0
//     if (epollfd == -1) {
//         std::cerr << "epoll_create unsuccessful --> exiting..." << std::endl;
//         perror(NULL);
//         return 1;
//     }
//     ev.events = EPOLLIN;
//     ev.data.fd = listener;
//     epoll_ctl(epollfd, EPOLL_CTL_ADD, listener, &ev);
//     // EPOLL_CTL_ADD: Adds a new file descriptor to the interest list.
//     // EPOLL_CTL_MOD: Modifies an existing file descriptor in the interest list.
//     // EPOLL_CTL_DEL: Removes a file descriptor from the interest list.

//     while(1) {
//         int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
//         if (nfds == -1) {
//             std::cerr << "epoll_wait unsuccessful --> exiting..." << std::endl;
//             return 1;
//         }
//         for (int rdy_fd = 0; rdy_fd < nfds; rdy_fd++) {
//             if (events[rdy_fd].data.fd == listener) {
//                 // rdy_fd is the server --> means we accept new client and add it to the epoll list
//                 int client_fd = accept(listener, NULL, NULL);
//                 if (client_fd == -1) {
//                     std::cerr << "accept unsuccessful --> exiting..." << std::endl;
//                     return 1;
//                 }
//                 fcntl(client_fd, F_SETFL, O_NONBLOCK);
//                 ev.events = EPOLLIN;
//                 ev.data.fd = client_fd;
//                 epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &ev);
//                 // ..........................
//             }
//             else { // rdy_fd is a client
//                 if (events[rdy_fd].events & EPOLLIN) { // rdy_fd is ready to read
//                     std::cout << "reading from client: " << events[rdy_fd].data.fd << std::endl;
//                     std::cout << "events: " << events[rdy_fd].events << std::endl;
//                     char buffer[1024];
//                     int bytes_read = recv(events[rdy_fd].data.fd, buffer, sizeof(buffer), 0);
//                     if (bytes_read == -1) {
//                         std::cerr << "recv unsuccessful --> exiting..." << std::endl;
//                         return 1;
//                     }
//                     else if (bytes_read == 0) {
//                         std::cout << "0 bytes read --> client disconnected --> DEL from epoll" << std::endl;
//                         epoll_ctl(epollfd, EPOLL_CTL_DEL, events[rdy_fd].data.fd, NULL);
//                         close(events[rdy_fd].data.fd);
//                     }
//                     else {
//                         std::cout << "events: " << events[rdy_fd].events << std::endl;
//                         std::cout << bytes_read << " bytes read from " << events[rdy_fd].data.fd << " --> assume we read the whole response "
//                                 "register client with EPOLLOUT (events)" << std::endl;
//                         events[rdy_fd].events |= EPOLLOUT;
//                         std::cout << "events: " << events[rdy_fd].events << std::endl;
//                         // send back to client
//                         //send(events[rdy_fd].data.fd, buffer, bytes_read, 0);
//                     }
//                 }
//                 if (events[rdy_fd].events & EPOLLOUT) { // rdy_fd is ready to write
//                     std::cout << "writing to client: " << events[rdy_fd].data.fd << std::endl;
//                     std::cout << "events: " << events[rdy_fd].events << std::endl;
//                     int bytes_send = send(events[rdy_fd].data.fd, http_response, http_response_len, 0);
//                     if (bytes_send == -1) {
//                         std::cerr << "send unsuccessful --> exiting..." << std::endl;
//                         return 1;
//                     }
//                     else {
//                         std::cout << bytes_send << " bytes sent" << std::endl;
//                         std::cout << "response has been sent to client --> remove from epoll" << std::endl;
//                         events[rdy_fd].events = EPOLLIN;
//                         // epoll_ctl(epollfd, EPOLL_CTL_DEL, events[rdy_fd].data.fd, NULL);
//                     }
//                 }
//             }
//         }
//     }
//     close(listener);
//     close(epollfd);

//     return 0;
// }
// ----------------------------------------------------
}

int EventManager::CheckWithSelect_()
{

    fd_set select_rd_set;
    fd_set select_wr_set;
    FD_ZERO(&select_rd_set);
    FD_ZERO(&select_wr_set);
    for (SockMapIt it = rd_sock_.begin(); it != rd_sock_.end(); ++it) {
        LOG(DEBUG) << "adding to select rd set: " << it->first;
        FD_SET(it->first, &select_rd_set);
    }
    for (SockMapIt it = wr_sock_.begin(); it != wr_sock_.end(); ++it) {
        FD_SET(it->first, &select_wr_set);
    }
    // rd_sock_ is never empty as long as at least 1 master socket exist
    int max_fd = rd_sock_.rbegin()->first;
    if (!wr_sock_.empty()) {
        max_fd = std::max(wr_sock_.rbegin()->first, max_fd);
    }
    struct timeval timeout = {10, 0};
    int num_of_fds =
        select(max_fd + 1, &select_rd_set, &select_wr_set, /* err fds */ NULL, &timeout);
    if (num_of_fds < 0) {
        // select errors or empty set, return error code?
        return 1;
    }
    // iterate over i here cuz call can change callbacks map
    for (int i = 0; i <= max_fd; ++i) {
        SockMapIt it;
        if (FD_ISSET(i, &select_rd_set) && ((it = rd_sock_.find(i)) != rd_sock_.end())) {
            it->second->Call(i); // receive
        }
        if (FD_ISSET(i, &select_wr_set) && ((it = wr_sock_.find(i)) != wr_sock_.end())) {
            it->second->Call(i); // send
        }
    }
    return 0;
}

int EventManager::RegisterReadCallback(int fd, utils::unique_ptr<c_api::EventManager::ICallback> callback)
{
    rd_sock_.insert(std::make_pair(fd, callback));
    return 0;
}

int EventManager::RegisterWriteCallback(int fd, utils::unique_ptr<c_api::EventManager::ICallback> callback)
{
    wr_sock_.insert(std::make_pair(fd, callback));
    return 0;
}

void EventManager::DeleteCallbacksByFd(int fd, CallbackType type)
{
    if (type & CT_READ) {
        rd_sock_.erase(fd);
    }
    if (type & CT_WRITE) {
        wr_sock_.erase(fd);
    }
}

}  // namespace c_api
