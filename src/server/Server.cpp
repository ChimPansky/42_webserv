#include "Server.h"
// #include "IProcessor.h"

Server::Server(const config::ServerConfig& cfg) : server_config_(cfg) {}

const std::string& Server::name() const { return server_config_.server_names()[0]; }

bool Server::DoesMatchTheRequest(const http::Request& /*rq*/) const
{
    return true;
}

// if returns nullptr, rs is the valid response right away
// utils::unique_ptr<IProcessor> AcceptRequest(const http::Request& rq, http::Response& rs)
// {
//     utils::unique_ptr<IProcessor> processor(new FileProcessor("www/index.html", rq, rs));

//     return processor;
// }

void Server::AcceptRequest(const http::Request& /*rq*/, utils::unique_ptr<http::IResponseCallback> cb) const {
    // const config::LocationConfig* chosen_loc = &locations_[0];  // choose location with method, host, uri, more?
    
    cb->Call(http::GetSimpleValidResponse());
}
