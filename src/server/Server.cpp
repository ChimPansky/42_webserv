#include "Server.h"
#include "IProcessor.h"

Server::Server(const config::ServerConfig& cfg) : server_config_(cfg) {}

const std::string& Server::name() const { return server_config_.server_names()[0]; }

bool Server::DoesMatchTheRequest(const http::Request& /*rq*/) const
{
    return true;
}

// if returns nullptr, rs is the valid response right away
utils::unique_ptr<IProcessor> AcceptRequest(const http::Request& rq, http::Response& rs)
{
    utils::unique_ptr<IProcessor> processor(new FileProcessor("www/index.html", rq, rs));

    return processor;
}


/*

register ClientRecvCallback with client session

ClientRecvCallback when rq is received

choose server for client

server Accept request -> ResponseBodyGenerator or response

if !response:

    register ResponseBodyGenerator callback with client session

    ResponseBodyGenerator callback when body is generated:

    response finalize

Client send response

register ClientSendCallback with response

*/