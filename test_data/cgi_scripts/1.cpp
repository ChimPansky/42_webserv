#include <fcntl.h>       // close
#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // socket, bind
#include <unistd.h>      // close

#include <cstring>
#include <sstream>    // close
#include <stdexcept>  // close
#include <string>     // close

// flags set with fcntl will be reset after fork, lol
bool set_flags(int sockfd_, int flags)
{
    int cur_flags = fcntl(sockfd_, F_GETFL);
    if (cur_flags < 0) {
        perror("fcntl");
        return false;
    }
    // int flags_to_set = (cur_flags ^ flags) & flags;
    // if (!flags_to_set) {
    //     return true;
    // }
    return (fcntl(sockfd_, F_SETFL, cur_flags | flags) >= 0);
}

int main(int, char**, char** env)
{
    int sockfd_ = ::socket(/* IPv4 */ AF_INET,
                           /* TCP */ SOCK_STREAM,
                           /* explicit tcp */ IPPROTO_TCP);
    if (sockfd_ < 0) {
        throw std::runtime_error("cannot create socket");
    }
    int optval = 1;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (!set_flags(sockfd_, O_NONBLOCK | O_CLOEXEC)) {
        perror("fcntl");
    }
    int pid = fork();
    if (pid == 0) {
        char* argv[] = {"/usr/bin/python3", "./1.py", NULL};
        execve("/usr/bin/python3", argv, env);
        perror("execve");
    }
    sleep(100);
}
