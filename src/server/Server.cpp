
#include "Server.h"
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
        LOG(DEBUG) << "RQ_GOOD -> Send Hello World";
        HelloWorldResponseProcessor tmp(cb);
    } else if (rq.status == http::RQ_INCOMPLETE) {
        throw std::logic_error("trying to accept incomplete rq");
    } else {
        LOG(DEBUG) << "RQ_BAD -> Send Error Response with " << rq.status;
        GeneratedErrorResponseProcessor tmp(cb, (http::ResponseCode)rq.status);
    }
}

std::pair<MatchType, std::string> Server::MatchHostName(
    const std::string& host, const std::vector<std::string>& server_names)
{
    typedef std::vector<std::string>::const_iterator NamesIter;
    std::pair<MatchType, std::string> best_match = std::make_pair(NO_MATCH, std::string());

    for (NamesIter it = server_names.begin(); it != server_names.end(); ++it) {
        std::string server_name = *it;

        if (host == server_name) {
            return std::make_pair(EXACT_MATCH, server_name);
        } else if (server_name[0] == '*' && host.size() >= server_name.size() &&
                   host.compare(host.size() - (server_name.size() - 1), server_name.size() - 1,
                                server_name.substr(1)) == 0) {
            best_match = std::make_pair(PREFIX_MATCH, server_name);
        } else if (server_name[server_name.size() - 1] == '*' &&
                   host.size() >= server_name.size() && server_name.size() > 2 &&
                   host.compare(0, server_name.size() - 1, server_name, 0,
                                server_name.size() - 1) == 0) {
            best_match = std::make_pair(SUFFIX_MATCH, server_name);
        }
    }
    return best_match;
}

std::string Server::GetInfo() const
{
    std::ostringstream oss;

    oss << "\n\t--Server information: --";
    oss << "\n\tAccess log path: " << access_log_path_;
    oss << "\n\tAccess log level: " << access_log_level_;
    oss << "\n\tError log path: " << error_log_path_;
    oss << "\n\tServer names:";

    for (size_t i = 0; i < server_names_.size(); i++) {
        oss << " " << server_names_[i];
    }
    /* for (size_t i = 0; i < locations_.size(); i++) {
        locations_[i].GetInfo();
    } */

    return oss.str();
}

std::pair<MatchType, std::string> Server::MatchedServerName(const http::Request& rq) const
{
    return MatchHostName(rq.GetHeaderVal("host").second, server_names_);
}
