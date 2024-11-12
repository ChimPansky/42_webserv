#include "Server.h"
#include "IResponseProcessor.h"

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

void Server::AcceptRequest(const http::Request& rq, utils::unique_ptr<http::IResponseCallback> cb) const {
    // const config::LocationConfig* chosen_loc = &locations_[0];  // choose location with method, host, uri, more?
    // 2 options: rq on creation if rs ready right away calls callback
    //      if not rdy register callback in event manager with client cb
    //  or response processor should be owned by client session
    if (rq.status == http::RQ_GOOD) {
        LOG(DEBUG) << "RQ_GOOD -> Send Hello World";
        HelloWorldResponseProcessor tmp(cb);
    } else if (rq.status == http::RQ_BAD) {
        LOG(DEBUG) << "RQ_BAD -> Send Error Response with " << rq.error_code;
        GeneratedErrorResponseProcessor tmp(cb, rq.error_code);
    } else {
        throw std::logic_error("trying to accept incomplete rq");
    }
}
