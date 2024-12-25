#include "Socket.h"

#include <fcntl.h>
#include <logger.h>
#include <netinet/in.h>
#include <unistd.h>

#include <stdexcept>

namespace c_api {

Socket::Socket(int fd) : sockfd_(fd)
{}

Socket::Socket(SockType type)
{
    if (type != ST_TCP_V4) {
        throw std::logic_error("Sock constructor is only implemented for TCP sockets");
    }
    sockfd_ = ::socket(/* IPv4 */ AF_INET,
                       /* TCP */ SOCK_STREAM | SOCK_NONBLOCK,
                       /* explicit tcp */ IPPROTO_TCP);
    if (sockfd_ < 0) {
        throw std::runtime_error("cannot create socket");
    }
}

bool Socket::TrySetFlags(int flags)
{
    int cur_flags = fcntl(sockfd_, F_GETFL);
    if (cur_flags < 0) {
        return false;
    }
    int flags_to_set = (cur_flags ^ flags) & flags;
    if (!flags_to_set) {
        return true;
    }
    return (fcntl(sockfd_, F_SETFL, cur_flags | flags_to_set) >= 0);
}

Socket::~Socket()
{
    close(sockfd_);
}

RecvPackage Socket::Recv(size_t read_size) const
{
    RecvPackage pack = {};
    ssize_t bytes_recvd =
        ::recv(sockfd_, (void*)buf_, std::min(read_size, SOCK_READ_BUF_SZ), MSG_NOSIGNAL);
    if (bytes_recvd < 0) {
        pack.status = RS_SOCK_ERR;
    } else if (bytes_recvd == 0) {
        pack.status = RS_SOCK_CLOSED;
    } else {
        pack.status = RS_OK;
        pack.data = buf_;
        pack.data_size = bytes_recvd;
    }
    return pack;
}

SockStatus Socket::Send(SendPackage& pack) const
{
    if (pack.AllDataSent()) {
        LOG(WARNING) << "redundant Send call";
        return RS_OK;
    }
    ssize_t bytes_sent = ::send(sockfd_, pack.buf.data() + pack.bytes_sent,
                                pack.buf.size() - pack.bytes_sent, MSG_NOSIGNAL);
    if (bytes_sent > 0) {
        pack.bytes_sent += bytes_sent;
        return RS_OK;
    }
    return (bytes_sent == 0 ? RS_SOCK_CLOSED : RS_SOCK_ERR);
}

SockStatus Socket::Send(SendFilePackage& pack) const
{
    if (pack.AllDataSent()) {
        LOG(WARNING) << "redundant Send call";
        return RS_OK;
    }
    if (!pack.PrepareNextChunk()) {
        return RS_SOCK_ERR;
    }
    return Send(pack.chunk());
}

utils::maybe<Socket::SocketPair> Socket::CreateLocalNonblockSocketPair()
{
    int socket_fds[2];

    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK,
                     /*default for unix sock*/ 0, socket_fds) < 0) {
        return utils::maybe_not();
    }
    return std::make_pair(utils::unique_ptr<Socket>(new Socket(socket_fds[0])),
                          utils::unique_ptr<Socket>(new Socket((socket_fds[1]))));
}

SendFilePackage::SendFilePackage(const char* path) : chunk_(SOCK_SEND_FILE_BUF_SZ), bytes_sent_(0)
{
    ifs_.open(path, std::ios::binary);
}

utils::unique_ptr<SendFilePackage> SendFilePackage::TryCreate(const char* path)
{
    utils::unique_ptr<SendFilePackage> pack(new SendFilePackage(path));
    if (pack->ifs_.is_open()) {
        return pack;
    }
    LOG(ERROR) << "Cannot open file for package: " << path;
    return utils::unique_ptr<SendFilePackage>();
}

bool SendFilePackage::PrepareNextChunk()
{
    if (!chunk_.AllDataSent()) {
        return true;
    }
    bytes_sent_ += chunk_.bytes_sent;
    if (ifs_.eof()) {
        chunk_.buf.clear();
        chunk_.bytes_sent = 0;
        return true;
    }
    chunk_.buf.resize(SOCK_SEND_FILE_BUF_SZ);
    ifs_.read(chunk_.buf.data(), SOCK_SEND_FILE_BUF_SZ);
    ssize_t read_size = ifs_.gcount();
    if (ifs_.fail() || ifs_.bad() || read_size < 0) {
        LOG(ERROR) << "Error reading file";
        return false;
    }
    chunk_.buf.resize(read_size);
    chunk_.bytes_sent = 0;
    return true;
}

bool SendFilePackage::AllDataSent() const
{
    return ifs_.eof() && chunk_.AllDataSent();
}


}  // namespace c_api
