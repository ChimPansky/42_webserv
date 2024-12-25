#include "Location.h"

Location::Location(const config::LocationConfig& cfg)
    : route_(cfg.route()),
      allowed_methods_(cfg.allowed_methods()),
      redirect_(cfg.redirect()),
      is_cgi_(cfg.is_cgi()),
      cgi_paths_(cfg.cgi_paths()),
      cgi_extensions_(cfg.cgi_extensions()),
      alias_dir_(cfg.alias_dir()),
      default_files_(cfg.default_files()),
      dir_listing_(cfg.dir_listing()),
      client_max_body_size_(cfg.client_max_body_size()),
      upload_dir_(cfg.upload_dir())
{}

const std::pair<std::string /*path*/, bool /*is_exact_match*/>& Location::route() const
{
    return route_;
}

const std::vector<http::Method>& Location::allowed_methods() const
{
    return allowed_methods_;
}

const std::pair<int /*status_code*/, std::string /*new_route*/>& Location::redirect() const
{
    return redirect_;
}

bool Location::is_cgi() const
{
    return is_cgi_;
}

const std::vector<std::string>& Location::cgi_paths() const
{
    return cgi_paths_;
}

const std::vector<std::string>& Location::cgi_extensions() const
{
    return cgi_extensions_;
}

const std::string& Location::alias_dir() const
{
    return alias_dir_;
}

const std::vector<std::string>& Location::default_files() const
{
    return default_files_;
}

bool Location::dir_listing() const
{
    return dir_listing_;
}


unsigned int Location::client_max_body_size() const
{
    return client_max_body_size_;
}

std::string Location::upload_dir() const
{
    return upload_dir_;
}

std::pair<std::string /*path_in_uri*/, bool /*is_exact_match*/> Location::MatchUriPath(
    const std::string& path, const std::pair<std::string, bool>& route)
{
    const std::string& route_path = route.first;
    bool is_exact_match = route.second;

    if (path == route_path) {
        return std::make_pair(route_path, is_exact_match);
    } else if (!is_exact_match && path.compare(0, route_path.size(), route_path) == 0) {
        return std::make_pair(route_path, false);
    }
    return std::make_pair(std::string(), false);
}

std::pair<std::string /*path_in_uri*/, bool /*is_exact_match*/> Location::MatchedRoute(
    const http::Request& rq) const
{
    return MatchUriPath(rq.rqTarget.path(), route_);
}

std::string Location::GetDebugString() const
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
        << "Alias directory: " << alias_dir() << "\n"
        << "Default files: ";
    for (size_t i = 0; i < default_files_.size(); ++i) {
        oss << "  " << default_files_[i];
    }
    oss << "\n"
        << "Client max body size: " << client_max_body_size() << " bytes\n";
    oss << "Directory listing: " << (dir_listing_ ? "enabled" : "disabled") << "\n";
    oss << "Upload Dir: " << upload_dir() << "\n";

    return oss.str();
}
