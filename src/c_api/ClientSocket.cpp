#include "ClientSocket.h"

#include <logger.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <vector>

namespace c_api {

ClientSocket::ClientSocket(int fd, sockaddr_in addr_in) : sock_(fd), addr_in_(addr_in)
{}

// technically at this point socket must be unbinded
//   probably with 'shutdown', which is not in the allowed funcs
//   otherwise socket will be close but port still occupied
//   untill kernel wont free it
//   search more
ClientSocket::~ClientSocket()
{
    /* shutdown(sockfd_, SHUT_RDWR); */
}

}  // namespace c_api
