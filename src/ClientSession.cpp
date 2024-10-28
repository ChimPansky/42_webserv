#include "ClientSession.h"

#include <EventManager.h>
#include <multiplexers/ICallback.h>
#include <RequestBuilder.h>
#include <logger.h>
#include <unique_ptr.h>
#include "ServerCluster.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd)
    : client_sock_(sock), master_socket_fd_(master_sock_fd), connection_closed_(false)
{
    // TODO move from here
    associated_server_ = ServerCluster::ChooseServer(master_socket_fd_, rq_builder_.rq() /*add here: rq_builder_.max_body_sz_*/);
    if (c_api::EventManager::get().RegisterCallback(
            client_sock_->sockfd(), c_api::CT_READ,
            utils::unique_ptr<c_api::ICallback>(new ClientReadCallback(*this))) != 0) {
        LOG(ERROR) << "Could not register read callback for client: " << client_sock_->sockfd();
        CloseConnection();
        return;
    }
}

ClientSession::~ClientSession()
{}

void ClientSession::CloseConnection()
{
    connection_closed_ = true;
    c_api::EventManager::get().DeleteCallback(client_sock_->sockfd(), c_api::CT_READWRITE);
    LOG(INFO) << "Client " << client_sock_->sockfd() << ": Connection closed";
}

bool ClientSession::connection_closed() const
{
    return connection_closed_;
}


// RECV REQUEST
void ClientSession::ProcessNewData(size_t bytes_recvd)
{
    rq_builder_.Build(bytes_recvd);
    // TODO:
    // turn this into a client callback to exclude dependency of builder on server
    // server cluster to singleton
    if (rq_builder_.builder_status() == http::RB_NEED_INFO_FROM_SERVER) {
        // associated_server_ = ServerCluster::ChooseServer(master_socket_fd_, rq_builder_.rq() /*add here: rq_builder_.max_body_sz_*/);
        rq_builder_.ApplyServerInfo(1000);  // then this
        rq_builder_.Build(bytes_recvd);     // and this are obsolete
    }
    // fuck the callback here
    if (rq_builder_.builder_status() == http::RB_DONE) {
        // TODO:
        // instead of deleting callback change status of session and read to /dev/null untill empty to get rid of bad data in case of error
        c_api::EventManager::get().DeleteCallback(client_sock_->sockfd(), c_api::CT_READ);
        rq_builder_.rq().Print();
        // server returns rs with basic headers and status complete/body generation in process + generator func
        rs_builder_ = associated_server_->AcceptRequest(rq_builder_.rq());
        if (rs_builder_->Complete()) {
            PrepareResponse();
        } else {
            c_api::EventManager::get().RegisterCallback(
                rs_builder_->bodygen_sock()->sockfd(), c_api::CT_READ,
                utils::unique_ptr<c_api::ICallback>(new ClientRsBodyGenCallback(*this, *rs_builder_)));
        }
    }
}


// SEND RESP
void ClientSession::PrepareResponse()
{
    if (c_api::EventManager::get().RegisterCallback(
            client_sock_->sockfd(), c_api::CT_WRITE,
            utils::unique_ptr<c_api::ICallback>(new ClientWriteCallback(*this, rs_builder_->GetResponse()->Dump()))) != 0) {
        LOG(ERROR) << "Could not register write callback for client: "
                    << client_sock_->sockfd();
        CloseConnection();
        return;
    }
}

void ClientSession::ResponseSentCleanup() {
    c_api::EventManager::get().DeleteCallback(client_sock_->sockfd(), c_api::CT_WRITE);
    // change state to read if keepalive or close connection
}


ClientSession::ClientReadCallback::ClientReadCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientReadCallback::Call(int /*fd*/)
{
    client_.rq_builder_.PrepareToRecvData(CLIENT_RD_CALLBACK_RD_SZ);
    ssize_t bytes_recvd =
        client_.client_sock_->Recv(client_.rq_builder_.buf(), CLIENT_RD_CALLBACK_RD_SZ);
    if (bytes_recvd < 0) {
        client_.CloseConnection();
        return;
    }
    client_.rq_builder_.AdjustBufferSize(bytes_recvd);
    client_.ProcessNewData(bytes_recvd);
}


ClientSession::ClientWriteCallback::ClientWriteCallback(ClientSession& client, std::vector<char> content) : client_(client), buf_(content), buf_send_idx_(0)
{}

void ClientSession::ClientWriteCallback::Call(int /*fd*/)
{
    // assert fd == client_sock.fd
    ssize_t bytes_sent = client_.client_sock_->Send(buf_, buf_send_idx_,
                                                    buf_.size() - buf_send_idx_);
    if (bytes_sent <= 0) {
        LOG(ERROR) << "error on send";  // add perror
        client_.CloseConnection();
        return;
    }
    if (buf_send_idx_ == buf_.size()) {
        LOG(INFO) << buf_send_idx_ << " bytes sent";
        client_.ResponseSentCleanup();
    }
}


ClientSession::ClientRsBodyGenCallback::ClientRsBodyGenCallback(ClientSession& client, http::Response::ResponseBuilder& rs_builder) :client_(client), rs_builder_(rs_builder) {
}

void ClientSession::ClientRsBodyGenCallback::Call(int /*fd*/) {
    ssize_t bytes_recvd = rs_builder_.bodygen_sock()->Recv();
    if (bytes_recvd < 0) {
        // change rs to 503?
    } else if (bytes_recvd == 0) {
        rs_builder_.Finalize();
        client_.PrepareResponse();
    } else {
        rs_builder_.AddToBody(bytes_recvd, rs_builder_.bodygen_sock()->buf());
    }
}
