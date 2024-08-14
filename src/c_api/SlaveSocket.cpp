#include "SlaveSocket.h"

#include <unistd.h>

namespace c_api {

SlaveSocket::SlaveSocket(int fd)
  : _sockfd(fd)
{}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
SlaveSocket::~SlaveSocket() {
	/* shutdown(_sockfd, SHUT_RDWR); */
    close(_sockfd);
}

int SlaveSocket::sockfd() const {
    return _sockfd;
}

}  // namespace c_api
