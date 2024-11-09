#include "Server.h"

#include <utility>

#include "IResponseProcessor.h"
#include "Request.h"

Server::Server(const config::ServerConfig& cfg)
    : access_log_path_(cfg.access_log_path()), access_log_level_(cfg.access_log_level()),
      error_log_path_(cfg.error_log_path()), server_names_(cfg.server_names()),
      locations_(cfg.locations())
{}

std::string Server::name() const
{
    if (server_names_.empty()) {
        return std::string();
    }
    return server_names_[0];
}

const std::string& Server::access_log_path() const
{
    return access_log_path_;
}

Severity Server::access_log_level() const
{
    return access_log_level_;
}

const std::string& Server::error_log_path() const
{
    return error_log_path_;
}

const std::vector<config::LocationConfig>& Server::locations() const
{
    return locations_;
}

// if returns nullptr, rs is the valid response right away
// utils::unique_ptr<IProcessor> AcceptRequest(const http::Request& rq, http::Response& rs)
// {
//     utils::unique_ptr<IProcessor> processor(new FileProcessor("www/index.html", rq, rs));

//     return processor;
// }

void Server::AcceptRequest(const http::Request& rq,
                           utils::unique_ptr<http::IResponseCallback> cb) const
{
    // const config::LocationConfig* chosen_loc = &locations_[0];  // choose location with method,
    // host, uri, more? 2 options: rq on creation if rs ready right away calls callback
    //      if not rdy register callback in event manager with client cb
    //  or response processor should be owned by client session
    if (rq.status == http::RQ_GOOD) {
        HelloWorldResponseProcessor tmp(cb);
    } else if (rq.status == http::RQ_BAD) {
        GeneratedErrorResponseProcessor tmp(cb, http::HTTP_BAD_REQUEST);
    } else {
        throw std::logic_error("trying to accept incomplete rq");
    }
}

std::pair<MatchType, std::string> Server::MatchedServerName(const http::Request& rq) const
{
    typedef std::vector<std::string>::const_iterator NamesIter;
    std::string host = rq.GetHeaderVal("host").second;

    for (NamesIter it = server_names_.begin(); it != server_names_.end(); ++it) {
        std::string server_name = *it;

        if (host == server_name) {
            return std::make_pair(EXACT_MATCH, server_name);
        } else if (server_name[0] == '*' && host.size() >= server_name.size() &&
                   host.compare(host.size() - (server_name.size() - 1), server_name.size() - 1,
                                server_name.substr(1)) == 0) {
            return std::make_pair(PREFIX_MATCH, server_name);
        } else if (server_name[server_name.size() - 1] == '*' &&
                   host.size() >= server_name.size() && server_name.size() > 2 &&
                   host.compare(0, server_name.size() - 1, server_name, 0,
                                server_name.size() - 1) == 0) {
            return std::make_pair(SUFFIX_MATCH, server_name);
        }
    }
    return std::make_pair(NO_MATCH, std::string());
}

void Server::Print() const
{
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "--Server information: --";
    LOG(DEBUG) << "Access log path: " << access_log_path_;
    LOG(DEBUG) << "Access log level: " << access_log_level_;
    LOG(DEBUG) << "Error log path: " << error_log_path_;
    LOG(DEBUG) << "Server names:";
    for (size_t i = 0; i < server_names_.size(); i++) {
        LOG(DEBUG) << "  " << server_names_[i];
    }
    for (size_t i = 0; i < locations_.size(); i++) {
        locations_[i].Print();
    }
}

// int main() {

//     std::cout << "Match:" << std::endl;
//     TestMatch("server1");  // should match
//     TestMatch("server2");  // should match
//     TestMatch("server3");  // should match

//     TestMatch("sub.sub.example.com");  // should match

//     // prefix match
//     TestMatch("www.example.com");  // should match
//     TestMatch("shop.example.com"); // should match
//     std::cout << std::endl << "Don't match:" << std::endl;
//     TestMatch("example.com");      // shouldn't match

//     TestMatch("server123");   // shouldn't match
//     TestMatch("serverx");     // shouldn't match
//     TestMatch("serversuffix");// shouldn't match

//     TestMatch("randomhost");  // shouldn't match
//     TestMatch("example.org"); // shouldn't match

//     TestMatch("");  // shouldn't match
//     TestMatch("wwwexample.com");  // shouldn't match
//     TestMatch("www.examplecom");  // shouldn't match

//     TestMatch("example.co");       // shouldn't match
//     TestMatch("sub.example.co");   // shouldn't match
// }
