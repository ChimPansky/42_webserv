#include "ClientSession.h"

#include <EventManager.h>
#include <RequestBuilder.h>
#include <ResponseCodes.h>
#include <Server.h>
#include <errors.h>
#include <logger.h>
#include <maybe.h>
#include <multiplexers/ICallback.h>
#include <shared_ptr.h>
#include <unique_ptr.h>

#include "ServerCluster.h"
#include "utils/utils.h"

ClientSession::ClientSession(utils::unique_ptr<c_api::ClientSocket> sock, int master_sock_fd,
                             utils::shared_ptr<Server> default_server)
    : client_sock_(sock),
      master_socket_fd_(master_sock_fd),
      rq_destination_(default_server),
      rq_builder_(utils::unique_ptr<http::IOnHeadersReadyCb>(new ChooseServerCb(*this))),
      connection_closed_(false),
      read_state_(CS_READ)
{
    UpdateLastActivityTime_();
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
    UpdateLastActivityTime_();
    rq_builder_.Build(pack.data, pack.data_size);
    if (rq_builder_.builder_status() == http::RB_DONE) {
        LOG(DEBUG) << "ProcessNewData: Done reading Request ("
                   << ((rq_builder_.rq().status == http::HTTP_OK) ? "GOOD)" : "BAD)")
                   << " -> Accept on Server...";
        read_state_ = CS_IGNORE;
        LOG(DEBUG) << rq_builder_.rq().GetDebugString();
        // server returns rs with basic headers and status complete/body generation in process +
        // generator func
        if (rq_destination_.server) {  // just to make sure we never dereference NULL...
            response_processor_ = rq_destination_.server->ProcessRequest(
                rq_builder_.rq(), rq_destination_,
                utils::unique_ptr<http::IResponseCallback>(
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
    LOG(DEBUG) << "Response:\n" << rs->GetDebugString();
    if (!c_api::EventManager::TryRegisterCallback(
            client_sock_->sockfd(), c_api::CT_WRITE,
            utils::unique_ptr<c_api::ICallback>(new OnReadyToSendToClientCb(
                *this, rs->Dump(), rs->body_file_path().c_str(), close_connection)))) {
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

void ClientSession::UpdateLastActivityTime_()
{
    last_activity_time_ = utils::Now();
}

///////////////////////////////////
//////////// CALLBACKS ////////////
///////////////////////////////////

http::HeadersValidationResult ClientSession::ChooseServerCb::Call(const http::Request& rq)
{
    RequestDestination& rq_dest = client_.rq_destination_;
    rq_dest.server = ServerCluster::ChooseServer(client_.master_socket_fd_, rq);
    std::pair<utils::shared_ptr<Location>, LocationType> chosen_loc =
        rq_dest.server->ChooseLocation(rq);
    if (chosen_loc.second == NO_LOCATION) {
        LOG(DEBUG) << "No location match -> 404";
        return http::HeadersValidationResult(http::HTTP_NOT_FOUND);
    }
    rq_dest.loc = chosen_loc.first;
    LOG(DEBUG) << "chosen loc: " << chosen_loc.first->GetDebugString();
    if (std::find(chosen_loc.first->allowed_methods().begin(),
                  chosen_loc.first->allowed_methods().end(),
                  rq.method) == chosen_loc.first->allowed_methods().end()) {
        LOG(DEBUG) << "Method not allowed for specific location -> 405";
        return http::HeadersValidationResult(http::HTTP_NOT_FOUND);
    }
    rq_dest.updated_path = utils::UpdatePath(chosen_loc.first->alias_dir(),
                                             chosen_loc.first->route().first, rq.rqTarget.path());

    http::HeadersValidationResult validation_result(http::HTTP_OK);
    if (chosen_loc.second == STATIC_PATH && rq.method == http::HTTP_POST) {
        validation_result.max_body_size = chosen_loc.first->client_max_body_size();
        validation_result.upload_path = rq_dest.updated_path;
    }
    return validation_result;
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
                                                                utils::maybe<const char*> file_body,
                                                                bool close_connection)
    : client_(client), pack_(content), close_after_sending_rs_(close_connection)
{
    if (file_body) {
        file_pack_ = c_api::SendFilePackage::TryCreate(*file_body);
        LOG_IF(ERROR, !file_pack_) << "Cannot create file package";
    }
}

void ClientSession::OnReadyToSendToClientCb::Call(int /*fd*/)
{
    LOG(DEBUG) << "OnReadyToSendToClientCb::Call";
    client_.UpdateLastActivityTime_();
    if (!pack_.AllDataSent()) {
        c_api::SockStatus status = client_.client_sock_->Send(pack_);
        if (status != c_api::RS_OK) {
            LOG_IF(ERROR, status == c_api::RS_SOCK_ERR)
                << "Error on send: " << utils::GetSystemErrorDescr();
            client_.CloseConnection();
            return;
        }
        if (!pack_.AllDataSent()) {
            return;
        } else {
            LOG(INFO) << pack_.bytes_sent << " bytes sent into " << client_.client_sock_->sockfd();
        }
    }
    if (file_pack_ && !file_pack_->AllDataSent()) {
        c_api::SockStatus status = client_.client_sock_->Send(*file_pack_);
        if (status != c_api::RS_OK) {
            LOG_IF(ERROR, status == c_api::RS_SOCK_ERR)
                << "Error on send: " << utils::GetSystemErrorDescr();
            client_.CloseConnection();
            return;
        }
        if (!file_pack_->AllDataSent()) {
            return;
        } else {
            LOG(INFO) << file_pack_->bytes_sent() << " bytes sent into "
                      << client_.client_sock_->sockfd();
        }
    }
    client_.ResponseSentCleanup(close_after_sending_rs_);
}

void ClientSession::ClientProceedWithResponseCallback::Call(utils::unique_ptr<http::Response> rs)
{
    client_.PrepareResponse(rs);
}
