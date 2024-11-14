#include <Location.h>

#include <sstream>

Location::Location()
    : route_(config::LocationConfig::kDefaultRoute()),
      allowed_methods_(config::LocationConfig::kDefaultAllowedMethods()), redirect_(0, ""),
      is_cgi_(false), cgi_paths_(), cgi_extensions_(),
      root_dir_(config::LocationConfig::kDefaultRootDir()),
      default_file_(config::LocationConfig::kDefaultIndexFile()),
      client_max_body_size_(config::LocationConfig::kDefaultClientMaxBodySize())
{}

Location::Location(const config::LocationConfig& cfg)
    : route_(cfg.route()), allowed_methods_(cfg.allowed_methods()), redirect_(cfg.redirect()),
      is_cgi_(cfg.is_cgi()), cgi_paths_(cfg.cgi_paths()), cgi_extensions_(cfg.cgi_extensions()),
      root_dir_(cfg.root_dir()), default_file_(cfg.default_file()),
      client_max_body_size_(cfg.client_max_body_size())
{}

const std::pair<std::string /* path */, bool /* is exact match */>& Location::route() const
{
    return route_;
}

const std::pair<int /* status code */, std::string /* new route */>& Location::redirect() const
{
    return redirect_;
}

const std::string& Location::root_dir() const
{
    return root_dir_;
}

const std::vector<std::string>& Location::default_file() const
{
    return default_file_;
}

unsigned int Location::client_max_body_size() const
{
    return client_max_body_size_;
}

std::pair<std::string /* path in uri */, bool /* is exact match */> Location::MatchUriPath(
    const std::string& path, const std::pair<std::string, bool>& route)
{
    const std::string& route_path = route.first;
    bool is_exact_match = route.second;

    if (path == route_path) {
        return std::make_pair(route_path, is_exact_match);
    } else if (!is_exact_match && path.compare(0, route_path.size(), route_path) == 0 &&
               path[route_path.size()] == '/') {
        return std::make_pair(route_path, false);
    }
    return std::make_pair(std::string(), false);
}

std::pair<std::string /* path in uri */, bool /* is exact match */> Location::MatchedRoute(
    const http::Request& rq) const
{
    return MatchUriPath(rq.uri.path(), route_);
}

std::string Location::GetInfo() const
{
    std::ostringstream oss;

    oss << "\n"
        << "--Location configuration: --\n"
        << "Route: " << route_.first << " " << (route_.second ? "(exact match)" : "(prefix match)")
        << "\n"
        << "Allowed methods: ";
    for (size_t i = 0; i < allowed_methods_.size(); ++i) {
        oss << "  " << allowed_methods_[i];
    }
    oss << "\n"
        << "Redirect: ";
    if (redirect_.first != 0) {
        oss << redirect_.first << " " << redirect_.second;
    } else {
        oss << "None";
    }
    oss << "\n"
        << "CGI: " << (is_cgi_ ? "enabled" : "disabled") << "\n"
        << "CGI paths: ";
    for (size_t i = 0; i < cgi_paths_.size(); ++i) {
        oss << "  " << cgi_paths_[i];
    }
    oss << "\n"
        << "CGI extensions: ";
    for (size_t i = 0; i < cgi_extensions_.size(); ++i) {
        oss << "  " << cgi_extensions_[i];
    }
    oss << "\n"
        << "Root directory: " << root_dir_ << "\n"
        << "Default files: ";
    for (size_t i = 0; i < default_file_.size(); ++i) {
        oss << "  " << default_file_[i];
    }
    oss << "\n"
        << "Client max body size: " << client_max_body_size_ << " bytes\n";

    return oss.str();
}

utils::unique_ptr<AResponseProcessor> Location::GetResponseProcessor(
    utils::unique_ptr<http::IResponseCallback> cb, const http::Request& rq) const
{
    if (is_cgi_) {
        // return utils::unique_ptr<AResponseProcessor>(new CgiResponseProcessor(cb, rq, cgi_paths,
        // cgi_extensions, root_dir));
    } else if (dir_listing_) {
        // return utils::unique_ptr<AResponseProcessor>(new DirListingResponseProcessor(cb, rq,
        // root_dir));
    } else if (rq.status == http::RQ_GOOD) {
        LOG(DEBUG) << "RQ_GOOD -> Send Hello World";
        return utils::unique_ptr<AResponseProcessor>(new HelloWorldResponseProcessor(cb));
    } else if (rq.status == http::RQ_INCOMPLETE) {
        throw std::logic_error("trying to accept incomplete rq");
    }
    LOG(DEBUG) << "RQ_BAD -> Send Error Response with " << rq.status;
    return utils::unique_ptr<AResponseProcessor>(
        new GeneratedErrorResponseProcessor(cb, (http::ResponseCode)rq.status));
}
