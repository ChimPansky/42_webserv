#include "Client.h"
#include "c_api/EventManager.h"
#include <iostream>

Client::Client(utils::unique_ptr<c_api::ClientSocket> sock)
  : _client_sock(sock), _buf_send_idx(0), _connection_closed(false)
{
    c_api::EventManager::get()
        .register_read_callback(_client_sock->sockfd(), utils::unique_ptr<utils::ICallback>(new ClientReadCallback(*this)));
}

Client::ClientReadCallback::ClientReadCallback(Client& c)
  : _client(c)
{}

int Client::ClientReadCallback::call(int) {
    // assert fd == client_sock.fd
    long bytes_recvdd = _client._client_sock->recv(_client._buf);
    if (bytes_recvdd <= 0) {
      // close connection
      _client._connection_closed = true;
      std::cout << "Connection closed" << std::endl;
      return 0;
    }
    std::cout << bytes_recvdd << " bytes recvd" << std::endl;
    std::cout.write(_client._buf.data(), _client._buf.size()) << std::flush;
    bytes_recvdd = _client._client_sock->send(_client._buf, _client._buf_send_idx, bytes_recvdd);
    return bytes_recvdd;
}
