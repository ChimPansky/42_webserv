#include "ClientSession.h"

#include "c_api/EventManager.h"
#include "utils/logger.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd)
    : client_sock_(sock),
      master_socket_fd_(master_sock_fd),
      buf_send_idx_(0),
      connection_closed_(false)
{
    c_api::EventManager::get().RegisterReadCallback(
        client_sock_->sockfd(),
        utils::unique_ptr<c_api::EventManager::ICallback>(new ClientReadCallback(*this)));
}

ClientSession::~ClientSession()
{
    c_api::EventManager::get().DeleteCallbacksByFd(client_sock_->sockfd());
}

bool ClientSession::connection_closed() const
{
    return connection_closed_;
}

#include <cstring>
#define HTTP_RESPONSE                                                                              \
    "HTTP/1.1 200 OK\r\n\
Date: Mon, 27 Jul 2009 12:28:53 GMT\n\r\
Server: ft_webserv\n\r\
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\n\r\
Content-Length: 88\n\r\
Content-Type: text/html\n\r\
Connection: Closed\n\r\
\n\r\
<html>\n\r\
<body>\n\r\
<h1>Hello, World!</h1>\n\r\
</body>\n\r\
</html>\n\r"

void ClientSession::ProcessNewData(ssize_t bytes_recvdd)
{
    rq_builder_.ParseChunk(buf_.data(), bytes_recvdd);
    LOG(DEBUG) << bytes_recvdd << " bytes recvd";
    LOG(DEBUG) << buf_.data();
    // std::cout.write(buf_.data(), buf_.size()) << std::flush;
    if (rq_builder_.IsRequestReady()) {
        // if cgi run and register callbacks for cgi
        // else return static page
        //rq_builder_.rq();

        c_api::EventManager::get().DeleteCallbacksByFd(client_sock_->sockfd(),
                                                       c_api::EventManager::CT_READ);
        buf_send_idx_ = 0;
        buf_.resize(sizeof(HTTP_RESPONSE));
        std::memcpy(buf_.data(), HTTP_RESPONSE, sizeof(HTTP_RESPONSE));
        c_api::EventManager::get().RegisterWriteCallback(
            client_sock_->sockfd(),
            utils::unique_ptr<c_api::EventManager::ICallback>(new ClientWriteCallback(*this)));
    }
}

ClientSession::ClientReadCallback::ClientReadCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientReadCallback::Call(int /*fd*/)
{
    // assert fd == client_sock.fd
    long bytes_recvdd = client_.client_sock_->Recv(client_.buf_);
    if (bytes_recvdd <= 0) {
        // close connection
        client_.connection_closed_ = true;

        LOG(INFO) << "Connection closed";
        return;
    }
    client_.ProcessNewData(bytes_recvdd);
}

ClientSession::ClientWriteCallback::ClientWriteCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientWriteCallback::Call(int /*fd*/)
{
    // assert fd == client_sock.fd
    ssize_t bytes_sent = client_.client_sock_->Send(client_.buf_, client_.buf_send_idx_,
                                                    client_.buf_.size() - client_.buf_send_idx_);
    if (bytes_sent <= 0) {
        // close connection
        client_.connection_closed_ = true;
        LOG(ERROR) << "error on send";  // add perror
        return;
    }
    if (client_.buf_send_idx_ == client_.buf_.size()) {
        client_.connection_closed_ = true;
        LOG(INFO) << client_.buf_send_idx_ << " bytes sent, connection closed";
    }
}
