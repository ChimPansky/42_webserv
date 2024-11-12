#include "ClientSession.h"

#include <EventManager.h>
#include <multiplexers/ICallback.h>
#include <RequestBuilder.h>
#include <logger.h>
#include <unique_ptr.h>
#include "ServerCluster.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd)
    : client_sock_(sock), master_socket_fd_(master_sock_fd), connection_closed_(false), read_state_(CS_READ)
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
void ClientSession::ProcessNewData(size_t bytes_recvd)
{
    rq_builder_.Build(bytes_recvd);
    if (rq_builder_.builder_status() == http::RB_NEED_TO_MATCH_SERVER) {
        LOG(DEBUG) << "ProcessNewData: Client " << client_sock_->sockfd() << " chooses Server on Mastersocket: " << master_socket_fd_;
        associated_server_ = ServerCluster::ChooseServer(master_socket_fd_, rq_builder_.rq() /*add here: rq_builder_.max_body_sz_*/);
        rq_builder_.ApplyServerInfo(1000);  // then this
        rq_builder_.Build(bytes_recvd);     // and this are obsolete
    }
    if (rq_builder_.builder_status() == http::RB_DONE) {
        LOG(DEBUG) << "ProcessNewData: Done reading Request (" << ((rq_builder_.rq().status == http::RQ_GOOD) ? "GOOD)" : "BAD)") << " -> Accept on Server...";
        read_state_ = CS_IGNORE;
        //rq_builder_.rq().Print();
        // server returns rs with basic headers and status complete/body generation in process + generator func
        if (associated_server_) { // just to make sure we never dereference NULL...
            associated_server_->AcceptRequest(rq_builder_.rq(), utils::unique_ptr<http::IResponseCallback>(new ClientProceedWithResponseCallback(*this)));
        }
    }
}


void ClientSession::PrepareResponse(utils::unique_ptr<http::Response> rs)
{
    if (c_api::EventManager::get().RegisterCallback(
            client_sock_->sockfd(), c_api::CT_WRITE,
            utils::unique_ptr<c_api::ICallback>(new ClientWriteCallback(*this, rs->Dump()))) != 0) {
        LOG(ERROR) << "Could not register write callback for client: "
                    << client_sock_->sockfd();
        CloseConnection();
    }
}

void ClientSession::ResponseSentCleanup() {
    c_api::EventManager::get().DeleteCallback(client_sock_->sockfd(), c_api::CT_WRITE);
    read_state_ = CS_READ;
}


ClientSession::ClientReadCallback::ClientReadCallback(ClientSession& client) : client_(client)
{}

void ClientSession::ClientReadCallback::Call(int /*fd*/)
{
    LOG(DEBUG) << "ClientReadCallback::Call";
    if (client_.read_state_ == CS_IGNORE) {
        std::vector<char> buf;
        buf.resize(1000);
        ssize_t bytes_recvd = client_.client_sock_->Recv(buf, 1000);
        LOG(DEBUG) << "RdCB::Call (ignored) bytes_recvd: " << bytes_recvd << " from: " << client_.client_sock_->sockfd();
        if (bytes_recvd <= 0) {
            LOG(DEBUG) << "Client disconnected -> terminating Session";
            client_.CloseConnection();
            return;
        }
    } else {
        client_.rq_builder_.PrepareToRecvData(CLIENT_RD_CALLBACK_RD_SZ);
        ssize_t bytes_recvd =
            client_.client_sock_->Recv(client_.rq_builder_.buf(), CLIENT_RD_CALLBACK_RD_SZ);
        // what u gonna do with the closed connection in builder?
        LOG(DEBUG) << "RdCB::Call (not ignored) bytes_recvd: " << bytes_recvd << " from: " << client_.client_sock_->sockfd();;
        // TODO: what if read-size == length of (invalid) request? 
        // we need to recv 0 bytes and forward it to builder in order to realize its bad:
        // read_size == 10 && rq == "GET / HTTP"
        if (bytes_recvd <= 0) {
            LOG(DEBUG) << "Client disconnected -> terminating Session";
            client_.CloseConnection();
            return;
        }
        client_.rq_builder_.AdjustBufferSize(bytes_recvd);
        client_.ProcessNewData(bytes_recvd);
    }
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


ClientSession::ClientProceedWithResponseCallback::ClientProceedWithResponseCallback(ClientSession& client) :client_(client) {
}

void ClientSession::ClientProceedWithResponseCallback::Call(utils::unique_ptr<http::Response> rs) {
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
