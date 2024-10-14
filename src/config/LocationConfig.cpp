#include "LocationConfig.h"

namespace config {

LocationConfig::LocationConfig(const std::pair<std::string, bool>& route,
                               const std::vector<Method>& allowed_methods,
                               const std::pair<int, std::string>& redirect,
                               const std::vector<std::string>& cgi_paths,
                               const std::vector<std::string>& cgi_extensions,
                               const std::string& root_dir,
                               const std::vector<std::string>& default_file, bool dir_listing,
                               unsigned int client_max_body_size)
    : route_(route), allowed_methods_(allowed_methods), redirect_(InitRedirect(redirect)),
      is_cgi_(route.first == "/cgi-bin/" || route.first == "/cgi-bin"), cgi_paths_(cgi_paths),
      cgi_extensions_(cgi_extensions), root_dir_(root_dir), default_file_(default_file),
      dir_listing_(dir_listing), client_max_body_size_(client_max_body_size)
{}

const std::pair<std::string, bool>& LocationConfig::route() const
{
    return route_;
}

const std::vector<LocationConfig::Method>& LocationConfig::allowed_methods() const
{
    return allowed_methods_;
}

const std::pair<int, std::string>& LocationConfig::redirect() const
{
    return redirect_;
}

bool LocationConfig::is_cgi() const
{
    return is_cgi_;
}

const std::vector<std::string>& LocationConfig::cgi_paths() const
{
    return cgi_paths_;
}

const std::vector<std::string>& LocationConfig::cgi_extensions() const
{
    return cgi_extensions_;
}

const std::string& LocationConfig::root_dir() const
{
    return root_dir_;
}

const std::vector<std::string>& LocationConfig::default_file() const
{
    return default_file_;
}

bool LocationConfig::dir_listing() const
{
    return dir_listing_;
}

unsigned int LocationConfig::client_max_body_size() const
{
    return client_max_body_size_;
}

std::pair<int, std::string> LocationConfig::InitRedirect(const std::pair<int, std::string>& value)
{
    if (value.first < 300 || value.first > 399) {
        throw std::runtime_error("Invalid configuration file: invalid redirect status code.");
    }
    return value;
}

void LocationConfig::Print() const
{
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "--Location configuration: --";
    LOG(DEBUG) << "Route: " << route_.first << " " << route_.second;
    LOG(DEBUG) << "Allowed methods: ";
    for (size_t i = 0; i < allowed_methods_.size(); i++) {
        LOG(DEBUG) << "  " << allowed_methods_[i];
    }
    LOG(DEBUG) << "Redirect: " << redirect_.first << " " << redirect_.second;
    LOG(DEBUG) << "CGI: " << (is_cgi_ ? "enabled" : "disabled");
    LOG(DEBUG) << "CGI paths: ";
    for (size_t i = 0; i < cgi_paths_.size(); i++) {
        LOG(DEBUG) << "  " << cgi_paths_[i];
    }
    LOG(DEBUG) << "CGI extensions: ";
    for (size_t i = 0; i < cgi_extensions_.size(); i++) {
        LOG(DEBUG) << "  " << cgi_extensions_[i];
    }
    LOG(DEBUG) << "Root directory: " << root_dir_;
    LOG(DEBUG) << "Default file: ";
    for (size_t i = 0; i < default_file_.size(); i++) {
        LOG(DEBUG) << "  " << default_file_[i];
    }
    LOG(DEBUG) << "Directory listing: " << dir_listing_;
    LOG(DEBUG) << "Client max body size: " << client_max_body_size_;
}

}  // namespace config
