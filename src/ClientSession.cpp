#include "ClientSession.h"

#include <EventManager.h>
#include <RequestBuilder.h>
#include <logger.h>
#include <multiplexers/ICallback.h>
#include <unique_ptr.h>

#include "ServerCluster.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd,
                             utils::shared_ptr<Server> default_server)
    : client_sock_(sock),
      master_socket_fd_(master_sock_fd),
      associated_server_(default_server),
      rq_builder_(utils::unique_ptr<http::IChooseServerCb>(new ChooseServerCb(*this))),
      connection_closed_(false),
      read_state_(CS_READ)
{
    UpdateLastActivitiTime_();
    if (!c_api::EventManager::TryRegisterCallback(
            client_sock_->sockfd(), c_api::CT_READ,
            utils::unique_ptr<c_api::ICallback>(new OnReadyToRecvFromClientCb(*this)))) {
        LOG(ERROR) << "Could not register read callback for client: " << client_sock_->sockfd();
        CloseConnection();
        return;
    }
}

ClientSession::~ClientSession()
{
    c_api::EventManager::DeleteCallback(client_sock_->sockfd(), c_api::CT_READWRITE);
}

void ClientSession::CloseConnection()
{
    connection_closed_ = true;
    LOG(INFO) << "Client " << client_sock_->sockfd() << ": Connection closed";
}

void ClientSession::ProcessNewData(c_api::RecvPackage& pack)
{
    UpdateLastActivitiTime_();
    rq_builder_.Build(pack.data, pack.data_size);
    if (rq_builder_.builder_status() == http::RB_DONE) {
        LOG(DEBUG) << "ProcessNewData: Done reading Request ("
                   << ((rq_builder_.rq().status == http::HTTP_OK) ? "GOOD)" : "BAD)")
                   << " -> Accept on Server...";
        read_state_ = CS_IGNORE;
        LOG(DEBUG) << rq_builder_.rq().GetDebugString();
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
    std::map<std::string, std::string>::const_iterator conn_it =
        rs->headers().find("Connection");  // TODO add find header case-independent
    bool close_connection = (conn_it != rs->headers().end() && conn_it->second == "Close");
    LOG(DEBUG) << "Sending rs from ClientSession with fd " << this->client_sock_->sockfd();
    LOG(DEBUG) << "Response:\n" << rs->DumpToStr();
    if (!c_api::EventManager::TryRegisterCallback(
            client_sock_->sockfd(), c_api::CT_WRITE,
            utils::unique_ptr<c_api::ICallback>(
                new OnReadyToSendToClientCb(*this, rs->Dump(), close_connection)))) {
        LOG(ERROR) << "Could not register write callback for client: " << client_sock_->sockfd();
        CloseConnection();
    }
}

void ClientSession::ResponseSentCleanup(bool close_connection)
{
    c_api::EventManager::DeleteCallback(client_sock_->sockfd(), c_api::CT_WRITE);
    read_state_ = CS_READ;
    if (close_connection) {
        CloseConnection();
    }
}

void ClientSession::UpdateLastActivitiTime_()
{
    last_activity_time_ = time(NULL);
}

///////////////////////////////////
//////////// CALLBACKS ////////////
///////////////////////////////////

http::ChosenServerParams ClientSession::ChooseServerCb::Call(const http::Request& rq)
{
    client_.associated_server_ = ServerCluster::ChooseServer(client_.master_socket_fd_, rq);
    http::ChosenServerParams params;
    std::pair<utils::shared_ptr<Location>, LocationType> chosen_loc =
        client_.associated_server_->ChooseLocation(rq);
    params.max_body_size =
        (chosen_loc.second != NO_LOCATION ? chosen_loc.first->client_max_body_size()
                                          : config::LocationConfig::kDefaultClientMaxBodySize());
    return params;
}

void ClientSession::OnReadyToRecvFromClientCb::Call(int /*fd*/)
{
    LOG(DEBUG) << "OnReadyToRecvFromClientCb::Call";
    if (client_.read_state_ != CS_IGNORE) {
        c_api::RecvPackage pack = client_.client_sock_->Recv();
        // what u gonna do with the closed connection in builder?
        LOG(DEBUG) << "RdCB::Call (not ignored) bytes_recvd: " << pack.data_size
                   << " from: " << client_.client_sock_->sockfd();
        // TODO: what if read-size == length of (invalid) request?
        // we need to recv 0 bytes and forward it to builder in order to realize its bad:
        // read_size == 10 && rq == "GET / HTTP"
        if (pack.status != c_api::RS_OK) {
            LOG(DEBUG) << "Client disconnected -> terminating Session";
            client_.CloseConnection();
            return;
        }
        client_.ProcessNewData(pack);
    }
}

ClientSession::OnReadyToSendToClientCb::OnReadyToSendToClientCb(ClientSession& client,
                                                                std::vector<char> content,
                                                                bool close_connection)
    : client_(client), pack_(content), close_after_sending_rs_(close_connection)
{}

void ClientSession::OnReadyToSendToClientCb::Call(int /*fd*/)
{
    LOG(DEBUG) << "OnReadyToSendToClientCb::Call";
    c_api::SockStatus status = client_.client_sock_->Send(pack_);
    if (status != c_api::RS_OK) {
        LOG(ERROR) << "error on send";  // TODO: add perror?
        client_.CloseConnection();
        return;
    }
    client_.UpdateLastActivitiTime_();
    if (pack_.AllDataSent()) {
        LOG(INFO) << pack_.bytes_sent << " bytes sent into " << client_.client_sock_->sockfd();
        client_.ResponseSentCleanup(close_after_sending_rs_);
    }
}

void ClientSession::ClientProceedWithResponseCallback::Call(utils::unique_ptr<http::Response> rs)
{
    client_.PrepareResponse(rs);
}
