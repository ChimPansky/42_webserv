#include "Client.h"
#include "c_api/EventManager.h"
#include <iostream>

Client::Client(utils::unique_ptr<c_api::ClientSocket> sock)
  : _client_sock(sock), _buf_send_idx(0), _connection_closed(false)
{
    c_api::EventManager::get()
        .RegisterReadCallback(_client_sock->sockfd(), utils::unique_ptr<utils::ICallback>(new ClientReadCallback(*this)));
}

Client::~Client() {
    c_api::EventManager::get()
        .DeleteCallbacksByFd(_client_sock->sockfd());
}

bool Client::connection_closed() const {
    return _connection_closed;
}

Client::ClientReadCallback::ClientReadCallback(Client& client)
  : _client(client)
{}

int Client::ClientReadCallback::Call(int /*fd*/) {
    // assert fd == client_sock.fd
    long bytes_recvdd = _client._client_sock->Recv(_client._buf);
    if (bytes_recvdd <= 0) {
      // close connection
      _client._connection_closed = true;
      std::cout << "Connection closed" << std::endl;
      return 0;
    }
    std::cout << bytes_recvdd << " bytes recvd" << std::endl;
    std::cout.write(_client._buf.data(), _client._buf.size()) << std::flush;
    bytes_recvdd = _client._client_sock->Send(_client._buf, _client._buf_send_idx, bytes_recvdd);
    return bytes_recvdd;
}
