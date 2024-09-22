#include "ClientSession.h"

#include "c_api/EventManager.h"
#include "c_api/multiplexers/ICallback.h"
#include "utils/logger.h"
#include "utils/unique_ptr.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd)
    : client_sock_(sock), master_socket_fd_(master_sock_fd), send_idx_(0),
      connection_closed_(false)
{
    if (c_api::EventManager::get().RegisterCallback(
        client_sock_->sockfd(), c_api::CT_READ,
        utils::unique_ptr<c_api::ICallback>(new ClientReadCallback(*this))) != 0) {
            LOG(ERROR) << "Could not register read callback for client: " << client_sock_->sockfd()
                << ". Closing client connection...";
            connection_closed_ = true;
            return ;
        }
}

ClientSession::~ClientSession()
{
    c_api::EventManager::get().DeleteCallback(client_sock_->sockfd(), c_api::CT_READWRITE);
}

bool ClientSession::connection_closed() const
{
    return connection_closed_;
}

#include <cstring>
#define HTTP_RESPONSE \
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

void ClientSession::PrepareResponse()
{
    send_idx_ = 0;
    client_buf_.resize(sizeof(HTTP_RESPONSE));
    std::memcpy(client_buf_.data(), HTTP_RESPONSE, sizeof(HTTP_RESPONSE));
}

ClientSession::ClientReadCallback::ClientReadCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientReadCallback::Call(int)
{
    // assert fd == client_sock.fd
    long bytes_recvd = client_.client_sock_->Recv(client_.client_buf_);
    if (bytes_recvd <= 0) {
        client_.connection_closed_ = true;
        LOG(INFO) << "Connection closed";
        return;
    }
    LOG(DEBUG) << "ClientReadCallback::Call: " << bytes_recvd << " bytes recvd from "
               << client_.client_sock_->sockfd();
    if (static_cast<size_t>(bytes_recvd) < client_.client_sock_->sock_buf_sz()) {
        c_api::EventManager::get().MarkCallbackForDeletion(client_.client_sock_->sockfd(),
                                                           c_api::CT_READ);
        if (c_api::EventManager::get().RegisterCallback(
            client_.client_sock_->sockfd(), c_api::CT_WRITE,
            utils::unique_ptr<c_api::ICallback>(new ClientWriteCallback(client_))) != 0) {
                LOG(ERROR) << "Could not register write callback for client: "
                    << client_.client_sock_->sockfd() << ". Closing connection...";
                client_.connection_closed_ = true;
                return ;
            }
        client_.PrepareResponse();
    }
}

ClientSession::ClientWriteCallback::ClientWriteCallback(ClientSession& client) : client_(client)
{
}

void ClientSession::ClientWriteCallback::Call(int)
{
    // assert fd == client_sock.fd
    ssize_t bytes_sent = client_.client_sock_->Send(client_.client_buf_, client_.send_idx_,
                                                    client_.client_buf_.size() - client_.send_idx_);
    if (bytes_sent <= 0) {
        client_.connection_closed_ = true;
        LOG(ERROR) << "error on send";  // add perror
        return;
    }
    if (client_.send_idx_ == client_.client_buf_.size()) {
        LOG(INFO) << client_.send_idx_
                  << " bytes sent, close connection";
        client_.connection_closed_ = true; // maybe keepalive - register readcallback
        c_api::EventManager::get().MarkCallbackForDeletion(client_.client_sock_->sockfd(),
                                                           c_api::CT_WRITE);
    }
}

// ClientSession::ProcessState ClientSession::ProcessRead(ssize_t bytes_recvd)
// {
//     (void)bytes_recvd;
//     LOG(DEBUG) << buf_.data();
//     // std::cout.write(buf_.data(), buf_.size()) << std::flush;
//     //TODO: set request is ready...
//     if (/*request is ready*/ buf_.size() > 20 ) {
//         // if cgi run and register callbacks for cgi
//         // else return static page

//         buf_send_idx_ = 0;
//         buf_.resize(sizeof(HTTP_RESPONSE));
//         std::memcpy(buf_.data(), HTTP_RESPONSE, sizeof(HTTP_RESPONSE));
//         return PS_DONE;
//     }
//     return PS_ONGOING;
// }
