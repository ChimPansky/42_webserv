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

#include "ServerCluster.h"
#include "c_api/EventManager.h"

#include <iostream>
#include <csignal>

void StopCluster(int /*signum*/) {
	ServerCluster::Stop();
	std::cout << " SIGINT caught, shutting down..." << std::endl;
}

int main(int ac, char **av) {
    trigger compilation error for workflow test!
	if (ac != 2) {
		std::cerr << "usage: ws <path-to-config-file>" << std::endl;
		return 1;
	}
	signal(SIGINT, StopCluster);

  c_api::EventManager::init(c_api::EventManager::MT_SELECT);

  ServerCluster::Start((Config(av[1])));  // curly braces is a dream
											 // another approach is Config::parse which returns config,
											 // but then copy c-tor for Configrequired, as RVO is not guaranteed
	return 0;
}
