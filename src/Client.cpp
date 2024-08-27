#include "Client.h"

#include <cstddef>
#include <iostream>

#include "c_api/EventManager.h"
#include "http/Request.h"

Client::Client(utils::unique_ptr<c_api::ClientSocket> sock)
    : _client_sock(sock), _buf_send_idx(0), _connection_closed(false)
{
    c_api::EventManager::get().RegisterReadCallback(
        _client_sock->sockfd(), utils::unique_ptr<utils::ICallback>(new ClientReadCallback(*this)));
}

Client::~Client()
{
    c_api::EventManager::get().DeleteCallbacksByFd(_client_sock->sockfd());
}

bool Client::connection_closed() const
{
    return _connection_closed;
}

const http::Request& Client::rq() const {
    return _rq;
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

void Client::ProcessNewData(ssize_t bytes_recvdd)
{
    std::cout << "\n" << bytes_recvdd << " bytes recvd" << std::endl;
    _rq.AddChunkToRequest(_client_sock->buf(), bytes_recvdd);
    if (IsRequestReady()) {
        std::cout << _rq.raw_request() << std::endl;

        // if cgi run and register callbacks for cgi
        // else return static page

        c_api::EventManager::get().DeleteCallbacksByFd(_client_sock->sockfd(),
                                                       c_api::EventManager::CT_READ);
        _buf_send_idx = 0;
        _buf.resize(sizeof(HTTP_RESPONSE));
        std::memcpy(_buf.data(), HTTP_RESPONSE, sizeof(HTTP_RESPONSE));
        c_api::EventManager::get().RegisterWriteCallback(
            _client_sock->sockfd(),
            utils::unique_ptr<utils::ICallback>(new ClientWriteCallback(*this)));
    }
}

Client::ClientReadCallback::ClientReadCallback(Client& client) : _client(client)
{}

void Client::ClientReadCallback::Call(int /*fd*/)
{
    // assert fd == client_sock.fd
    long bytes_recvdd = _client._client_sock->Recv();  // server receives from client connection --> store in ClientSocket-Buffer
    if (bytes_recvdd <= 0) {
        // close connection
        _client._connection_closed = true;
        std::cout << "Connection closed" << std::endl;
        return;
    }
    _client.ProcessNewData(bytes_recvdd);
}

Client::ClientWriteCallback::ClientWriteCallback(Client& client) : _client(client)
{}

void Client::ClientWriteCallback::Call(int /*fd*/)
{
    // assert fd == client_sock.fd
    ssize_t bytes_sent = _client._client_sock->Send(_client._buf, _client._buf_send_idx,
                                                    _client._buf.size() - _client._buf_send_idx);
    if (bytes_sent <= 0) {
        // close connection
        _client._connection_closed = true;
        std::cerr << "error on send" << std::endl;
        return;
    }
    if (_client._buf_send_idx == _client._buf.size()) {
        _client._connection_closed = true;
        std::cout << _client._buf_send_idx << " bytes sent, connection closed" << std::endl;
    }
}

bool Client::IsRequestReady() const
{
    std::cout << "Checking if request is ready (looking for http::EOF...)" << std::endl;
    size_t rq_size = _rq.raw_request().size();
    return (rq_size >= 4 && _rq.raw_request().substr(rq_size - 4) == http::httpEOF);
}
