#include "ClientSession.h"

#include "c_api/EventManager.h"
#include "utils/logger.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd)
    : client_sock_(sock),
      master_socket_fd_(master_sock_fd),
      send_idx_(0),
      connection_closed_(false)
{
    c_api::EventManager::get().RegisterCallback(
        client_sock_->sockfd(),
        utils::unique_ptr<c_api::EventManager::ICallback>(new ClientCallback(*this)));
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


ClientSession::ClientCallback::ClientCallback(ClientSession& client) : client_(client)
{
    callback_mode_ = c_api::EventManager::CM_READ;  // clients always start in read mode
    added_to_multiplex_ = false; // will be set to true once it is added to epoll
}

void ClientSession::ClientCallback::Call(int /*fd*/)
{
    if (callback_mode_ == c_api::EventManager::CM_READ) {
        ReadCall();
    }
    else if (callback_mode_ == c_api::EventManager::CM_WRITE) {
        WriteCall();
    }
}


c_api::EventManager::CallbackMode ClientSession::ClientCallback::callback_mode()
{
    return callback_mode_;
}

bool ClientSession::ClientCallback::added_to_multiplex()
{
    return added_to_multiplex_;
}

void ClientSession::ClientCallback::set_added_to_multiplex(bool added)
{
    added_to_multiplex_ = added;
}


void ClientSession::ClientCallback::ReadCall()
{
    // assert fd == client_sock.fd
    long bytes_recvd = client_.client_sock_->Recv(client_.client_buf_);
    if (bytes_recvd <= 0) {
        // close connection
        client_.connection_closed_ = true;
        LOG(INFO) << "Connection closed";
        return;
    }
    LOG(DEBUG) << "ClientCallback::ReadCall: " << bytes_recvd << " bytes recvd from " << client_.client_sock_->sockfd();
    client_.rq_builder_.ParseNext(client_.client_buf_);
    if (client_.rq_builder_.is_ready_for_response() || static_cast<size_t>(bytes_recvd) < client_.client_sock_->sock_buf_sz()) {
        LOG(DEBUG) << "ClientCallback::ReadCall: switching to write_mode";
        callback_mode_ = c_api::EventManager::CM_WRITE; // switch to write mode after Request reading is finished (and processed by server)
        // todo: pick server and process request
        client_.PrepareResponse();
    }
}

void ClientSession::PrepareResponse() {
    LOG(DEBUG) << "ClientSession::PrepareResponse";
    client_buf_.clear();
    client_buf_.resize(sizeof(HTTP_RESPONSE));
    send_idx_ = 0;
    std::memcpy(client_buf_.data(), HTTP_RESPONSE, sizeof(HTTP_RESPONSE));
}

void ClientSession::ClientCallback::WriteCall()
{
    LOG(DEBUG) << "ClientCallback::WriteCall";
        // assert fd == client_sock.fd
    ssize_t bytes_sent = client_.client_sock_->Send(client_.client_buf_, client_.send_idx_,
                                                    client_.client_buf_.size() - client_.send_idx_);
    if (bytes_sent <= 0) {
        // close connection
        client_.connection_closed_ = true;
        LOG(ERROR) << "error on send";  // add perror
        return;
    }
     if (client_.send_idx_ == client_.client_buf_.size()) {
        LOG(INFO) << client_.send_idx_ << " bytes sent, close connection (later: check keepalive and mb wait for next request)";
        client_.connection_closed_ = true;
        callback_mode_ = c_api::EventManager::CM_DELETE; // maybe keepalive - switch back to read mode CM_READ
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
