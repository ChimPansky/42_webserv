#include "ClientSession.h"

#include <EventManager.h>
#include <RequestBuilder.h>
#include <logger.h>
#include <multiplexers/ICallback.h>
#include <unique_ptr.h>

#include "ServerCluster.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd,
                             utils::shared_ptr<Server> default_server)
    : client_sock_(sock), master_socket_fd_(master_sock_fd), associated_server_(default_server),
      connection_closed_(false), read_state_(CS_READ)
{
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
void ClientSession::ProcessNewData(std::vector<char>& buf)
{
    if (buf.empty()) {
        LOG(DEBUG) << "ProcessNewData: Empty buffer -> ignoring";
        return;
    }
    rq_.ProcessNewData(buf); // build request until after-headers or append buf to body
    if (rq_.status() == http::RQ_END_OF_HEADERS_NOT_REACHED) {
        LOG(DEBUG) << "ProcessNewData: End of header-section not reached -> waiting for more data";
        return;
    }
    if (rq_.status() == http::RQ_END_OF_HEADERS_REACHED && rq_.BodyExpected()
        && !rq_.body().ready_to_send_to_server()) {
        LOG(DEBUG) << "ProcessNewData: Body / Bodychunk not complete -> waiting for more data";
        return;
    }
    if (!associated_server_) {
        associated_server_ = ServerCluster::ChooseServer(master_socket_fd_, rq_);
    }

    // todo: send rq to server and server will either send response right away (for non-POST methods)
    // OR attempt to write data from request-body to location on server
    // therefore need to change Server::ProcessRequest()...

    //associated_server_->ProcessRequest(rq_);
    //...............

    if (rq_builder_.builder_status() == http::RB_DONE) {
        LOG(DEBUG) << "ProcessNewData: Done reading Request ("
                   << ((rq_builder_.rq().status == http::RQ_GOOD) ? "GOOD)" : "BAD)")
                   << " -> Accept on Server...";
        read_state_ = CS_IGNORE;
        LOG(DEBUG) << rq_builder_.rq().ToString();
        // server returns rs with basic headers and status complete/body generation in process +
        // generator func
        if (associated_server_) {  // just to make sure we never dereference NULL...
            response_processor_ = associated_server_->ProcessRequest(
                rq_builder_.rq(), utils::unique_ptr<http::IResponseCallback>(
                                      new ClientProceedWithResponseCallback(*this)));
        } else {
            throw std::logic_error("trying to accept forward rq to non-existent server");
        }
    }
}


void ClientSession::PrepareResponse(utils::unique_ptr<http::Response> rs)
{
    ServerCluster::FillResponseHeaders(*rs);
    std::map<std::string, std::string>::const_iterator conn_it = rs->headers().find("connection");
    bool close_connection = (conn_it != rs->headers().end() && conn_it->second == "Close");
    if (c_api::EventManager::get().RegisterCallback(
            client_sock_->sockfd(), c_api::CT_WRITE,
            utils::unique_ptr<c_api::ICallback>(
                new ClientWriteCallback(*this, rs->Dump(), close_connection))) != 0) {
        LOG(ERROR) << "Could not register write callback for client: " << client_sock_->sockfd();
        CloseConnection();
    }
}

void ClientSession::ResponseSentCleanup(bool close_connection)
{
    c_api::EventManager::get().DeleteCallback(client_sock_->sockfd(), c_api::CT_WRITE);
    read_state_ = CS_READ;
    if (close_connection) {
        connection_closed_ = true;
    }
}


ClientSession::ClientReadCallback::ClientReadCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientReadCallback::Call(int /*fd*/)
{
    LOG(DEBUG) << "ClientReadCallback::Call";
    std::vector<char> buf;
    buf.resize(CLIENT_RD_CALLBACK_RD_SZ);
    ssize_t bytes_recvd = client_.client_sock_->Recv(buf, CLIENT_RD_CALLBACK_RD_SZ);
    if (bytes_recvd <= 0) {
        LOG(DEBUG) << "Client disconnected -> terminating Session";
        client_.CloseConnection();
        return;
    }
    if (client_.read_state_ == CS_IGNORE) {
        LOG(DEBUG) << "RdCB::Call (ignored) bytes_recvd: " << bytes_recvd
                   << " from: " << client_.client_sock_->sockfd();
    } else {
        LOG(DEBUG) << "RdCB::Call (not ignored) bytes_recvd: " << bytes_recvd
                   << " from: " << client_.client_sock_->sockfd();
        client_.ProcessNewData(buf);
    }
}


ClientSession::ClientWriteCallback::ClientWriteCallback(ClientSession& client,
                                                        std::vector<char> content,
                                                        bool close_connection)
    : client_(client), buf_(content), buf_send_idx_(0), close_after_sending_rs_(close_connection)
{}

void ClientSession::ClientWriteCallback::Call(int /*fd*/)
{
    LOG(DEBUG) << "ClientWriteCallback::Call";
    // assert fd == client_sock.fd
    ssize_t bytes_sent =
        client_.client_sock_->Send(buf_, buf_send_idx_, buf_.size() - buf_send_idx_);
    if (bytes_sent <= 0) {
        LOG(ERROR) << "error on send";  // add perror
        client_.CloseConnection();
        return;
    }
    if (buf_send_idx_ == buf_.size()) {
        LOG(INFO) << buf_send_idx_ << " bytes sent";
        client_.ResponseSentCleanup(close_after_sending_rs_);
    }
}


ClientSession::ClientProceedWithResponseCallback::ClientProceedWithResponseCallback(
    ClientSession& client)
    : client_(client)
{}

void ClientSession::ClientProceedWithResponseCallback::Call(utils::unique_ptr<http::Response> rs)
{
    LOG(DEBUG) << "ClientProceedWithResponseCallback::Call -> client_.PrepareResponse";
    client_.PrepareResponse(rs);
    // ssize_t bytes_recvd = rs_builder_.bodygen_sock()->Recv();
    // if (bytes_recvd < 0) {
    //     // change rs to 503?
    // } else if (bytes_recvd == 0) {
    //     rs_builder_.Finalize();
    // } else {
    //     rs_builder_.AddToBody(bytes_recvd, rs_builder_.bodygen_sock()->buf());
    // }
}
