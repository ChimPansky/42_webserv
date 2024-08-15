#include "Client.h"
#include "c_api/EventManager.h"

Client::Client(utils::unique_ptr<c_api::ClientSocket> sock)
  : _client_sock(sock)
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
    size_t idx = 0;
    bytes_recvdd = _client._client_sock->send(_client._buf, idx, bytes_recvdd);
    return bytes_recvdd;
}
