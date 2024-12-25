// allowed function:
// execve, dup, dup2, pipe, strerror, gai_strerror,
// errno, dup, dup2, fork, socketpair, htons, htonl,
// ntohs, ntohl, select, poll, epoll (epoll_create, epoll_ctl, epoll_wait),
// kqueue (kqueue, kevent),
// socket, accept, listen, send, recv, chdir bind,
// connect, getaddrinfo, freeaddrinfo, setsockopt,
// getsockname, getprotobyname, fcntl, close, read,
// write, waitpid, kill, signal, access, stat, open,
// opendir, readdir and closedir
// just some comment to trigger valgrind CI on github


#include <ConfigBuilder.h>

#include <iostream>

#include "ServerCluster.h"


int main(int ac, char** av)
try {
    if (ac != 2) {
        LOG(ERROR) << "usage: webserv <path-to-config-file>";
        return 1;
    }

    config::Config cfg = config::ConfigBuilder::GetConfigFromConfFile(av[1]);
    ServerCluster::Init(cfg);
    ServerCluster::Run();

    return 0;
} catch (const std::exception& e) {
    std::cerr
        << "Something went wrong and instead of creating a useful dump we're just showing this: "
        << e.what() << std::endl;
}
