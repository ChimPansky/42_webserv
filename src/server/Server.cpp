
#include "Server.h"

#include "Request.h"
#include "shared_ptr.h"

Server::Server(const config::ServerConfig& cfg)
    : access_log_path_(cfg.access_log_path()), access_log_level_(cfg.access_log_level()),
      error_log_path_(cfg.error_log_path()), server_names_(cfg.server_names())
{
    typedef std::vector<config::LocationConfig>::const_iterator LocationConfigConstIt;

    for (LocationConfigConstIt it = cfg.locations().begin(); it != cfg.locations().end(); ++it) {
        locations_.push_back(utils::shared_ptr<Location>(new Location(*it)));
    }
}

std::string Server::name() const
{
    if (server_names_.empty()) {
        return std::string();
    }
    return server_names_[0];
}

const std::vector<std::string>& Server::server_names() const
{
    return server_names_;
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

const std::vector<utils::shared_ptr<Location> >& Server::locations() const
{
    return locations_;
}

utils::shared_ptr<Location> Server::ChooseLocation(const http::Request& rq) const
{
    std::pair<std::string /*route*/, bool /*is_exact_match*/> best_match(std::string(), false);
    utils::shared_ptr<Location> matched_location;

    for (LocationsConstIt it = locations_.begin(); it != locations_.end(); ++it) {
        std::pair<std::string, bool> match_result = (*it)->MatchedRoute(rq);

        if ((match_result.first.length() > best_match.first.length() &&
             match_result.second == best_match.second) ||
            match_result.second > best_match.second) {
            best_match = match_result;
            matched_location = *it;
        }
    }
    return (best_match.first.empty() ? utils::shared_ptr<Location>(NULL) : matched_location);
}

// if returns nullptr, rs is the valid response right away
// utils::unique_ptr<IProcessor> AcceptRequest(const http::Request& rq, http::Response& rs)
// {
//     utils::unique_ptr<IProcessor> processor(new FileProcessor("www/index.html", rq, rs));

//     return processor;
// }

utils::unique_ptr<AResponseProcessor> Server::ProcessRequest(
    const http::Request& rq, utils::unique_ptr<http::IResponseCallback> cb) const
{
    switch (rq.status) {
        case http::RQ_INCOMPLETE:
            throw std::logic_error("trying to accept incomplete rq");
        case http::RQ_BAD:
            LOG(DEBUG) << "RQ_BAD -> Send Error Response with " << rq.status;
            return utils::unique_ptr<AResponseProcessor>(
                new GeneratedErrorResponseProcessor(cb, (http::ResponseCode)rq.status));
        case http::RQ_URI_TOO_LONG:
            LOG(DEBUG) << "RQ_BAD -> Send Error Response with " << rq.status;
            return utils::unique_ptr<AResponseProcessor>(
                new GeneratedErrorResponseProcessor(cb, (http::ResponseCode)rq.status));
        case http::RQ_GOOD:
            return GetResponseProcessor(rq, cb);
    }
    throw std::logic_error("unknown rq status");  // temporary - to silence compiler
}

utils::unique_ptr<AResponseProcessor> Server::GetResponseProcessor(
    const http::Request& rq, utils::unique_ptr<http::IResponseCallback> cb) const
{
    const utils::shared_ptr<Location> chosen_loc =
        ChooseLocation(rq);  // choose location with method,
    // host, uri, more? 2 options: rq on creation if rs ready right away calls callback
    //      if not rdy register callback in event manager with client cb
    //  or response processor should be owned by client session
    if (!chosen_loc) {
        LOG(DEBUG) << "RQ_BAD -> Send Error Response with " << http::HTTP_NOT_FOUND;
        return utils::unique_ptr<AResponseProcessor>(
            new GeneratedErrorResponseProcessor(cb, http::HTTP_NOT_FOUND));
    } else if (chosen_loc->is_cgi()) {
        // return utils::unique_ptr<AResponseProcessor>(new CgiResponseProcessor(cb, rq, cgi_paths,
        // cgi_extensions, root_dir));
    } else if (chosen_loc->dir_listing()) {
        // return utils::unique_ptr<AResponseProcessor>(new DirListingResponseProcessor(cb, rq,
        // root_dir));
    }
    LOG(DEBUG) << "RQ_GOOD -> Send Hello World";
    return utils::unique_ptr<AResponseProcessor>(new HelloWorldResponseProcessor(cb));
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

std::pair<MatchType, std::string> Server::MatchedServerName(const http::Request& rq) const
{
    return MatchHostName(rq.GetHeaderVal("host").second, server_names_);
}

std::string Server::GetDebugString() const
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
    for (size_t i = 0; i < locations().size(); i++) {
        oss << locations_[i]->GetDebugString();
    }
    return oss.str();
}

void Server::FillResponseHeaders(http::Response& rs) const
{
    std::map<std::string, std::string> headers = rs.headers();

    headers["Server"] = "ft_webserv";
    headers["Date"] = utils::GetFormatedTime();

    rs.set_headers(headers);
}
