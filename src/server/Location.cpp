#include "Location.h"

#include "LocationConfig.h"

Location::Location()
    : route_("/", false), allowed_methods_(config::LocationConfig::kDefaultAllowedMethods()),
      redirect_(0, ""), is_cgi_(false), cgi_paths_(), cgi_extensions_(),
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

std::pair<bool, std::string> Location::MatchedRoute(const http::Request& rq) const
{
    std::string path = rq.uri;
    std::string best_match = route_.first;

    if (path == route_.first && route_.second) {
        return std::make_pair(true, route_.first);
    } else if (path == route_.first && !route_.second) {
        return std::make_pair(false, route_.first);
    } else if (path.compare(0, route_.first.size(), route_.first) == 0) {
        return std::make_pair(false, route_.first);
    }
    return std::make_pair(false, std::string());
}
