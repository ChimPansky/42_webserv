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

#include <csignal>

#include "ServerCluster.h"
#include "utils/logger.h"

void StopCluster(int /*signum*/)
{
    LOG(INFO) << " SIGINT caught, shutting down...";
    ServerCluster::Stop();
}

int main(int ac, char **av)
{
    if (ac != 2) {
        LOG(ERROR) << "usage: ws <path-to-config-file>";
        return 1;
    }
    signal(SIGINT, StopCluster);

    ServerCluster::Start((Config(av[1])));  // curly braces is a dream
                        // another approach is Config::parse which returns config,
                        // but then copy c-tor for Configrequired, as RVO is not guaranteed
    return 0;
}
