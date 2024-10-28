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

utils::unique_ptr<http::Response::ResponseBuilder> Server::AcceptRequest(const http::Request& /*rq*/) const {
    // find location, open file/cgi/generate error response
    return utils::unique_ptr<http::Response::ResponseBuilder>(new http::Response::ResponseBuilder(
         utils::unique_ptr<http::Response>(new http::Response(http::OK, std::vector<char>()))));
}
