#include "ClientSession.h"

#include <iostream>

#include "c_api/EventManager.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock)
    : _client_sock(sock), _buf_send_idx(0), _connection_closed(false)
{
    c_api::EventManager::get().RegisterReadCallback(
        _client_sock->sockfd(), utils::unique_ptr<utils::ICallback>(new ClientReadCallback(*this)));
}

ClientSession::~ClientSession()
{
    c_api::EventManager::get().DeleteCallbacksByFd(_client_sock->sockfd());
}

bool ClientSession::connection_closed() const
{
    return _connection_closed;
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
    std::cout << bytes_recvdd << " bytes recvd" << std::endl;
    std::cout.write(buf_.data(), buf_.size()) << std::flush;
    if (/*request is ready*/ buf_.size() > 20) {
        // if cgi run and register callbacks for cgi
        // else return static page

        c_api::EventManager::get().DeleteCallbacksByFd(_client_sock->sockfd(),
                                                       c_api::EventManager::CT_READ);
        _buf_send_idx = 0;
        buf_.resize(sizeof(HTTP_RESPONSE));
        std::memcpy(buf_.data(), HTTP_RESPONSE, sizeof(HTTP_RESPONSE));
        c_api::EventManager::get().RegisterWriteCallback(
            _client_sock->sockfd(),
            utils::unique_ptr<utils::ICallback>(new ClientWriteCallback(*this)));
    }
}

ClientSession::ClientReadCallback::ClientReadCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientReadCallback::Call(int /*fd*/)
{
    // assert fd == client_sock.fd
    long bytes_recvdd = client_._client_sock->Recv(client_.buf_);
    if (bytes_recvdd <= 0) {
        // close connection
        client_._connection_closed = true;
        std::cout << "Connection closed" << std::endl;
        return;
    }
    client_.ProcessNewData(bytes_recvdd);
}

ClientSession::ClientWriteCallback::ClientWriteCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientWriteCallback::Call(int /*fd*/)
{
    // assert fd == client_sock.fd
    ssize_t bytes_sent = client_._client_sock->Send(client_.buf_, client_._buf_send_idx,
                                                    client_.buf_.size() - client_._buf_send_idx);
    if (bytes_sent <= 0) {
        // close connection
        client_._connection_closed = true;
        std::cerr << "error on send" << std::endl;
        return;
    }
    if (client_._buf_send_idx == client_.buf_.size()) {
        client_._connection_closed = true;
        std::cout << client_._buf_send_idx << " bytes sent, connection closed" << std::endl;
    }
}
