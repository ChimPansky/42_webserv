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

std::pair<bool, std::string> Location::MatchUriPath(const std::string& path,
                                                    const std::pair<std::string, bool>& route)
{
    if (path == route.first && route.second) {
        return std::make_pair(true, route.first);
    } else if (path == route.first && !route.second) {
        return std::make_pair(false, route.first);
    } else if (path.compare(0, route.first.size(), route.first) == 0 && !route.second) {
        return std::make_pair(false, route.first);
    }
    return std::make_pair(false, std::string());
}

std::pair<bool, std::string> Location::MatchedRoute(const http::Request& rq) const
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
