#include "LocationConfig.h"

namespace config {

bool IsCgiLocation(const std::string& route)
{
    const std::string cgi_prefix = "/cgi-bin/";

    size_t cgi_bin_pos = route.find(cgi_prefix);
    if (cgi_bin_pos == std::string::npos) {
        return false;
    }

    size_t next_char_pos = cgi_bin_pos + cgi_prefix.length();
    if (route[next_char_pos] == '\0') {
        return true;
    }
    return false;
}

LocationConfig::LocationConfig(
    const std::pair<std::string /*path*/, bool /*is_exact_match*/>& route,
    const std::vector<http::Method>& allowed_methods,
    const std::pair<int /*status_code*/, std::string /*new_route*/>& redirect,
    const std::vector<std::string>& cgi_extensions, const std::string& alias_dir,
    const std::vector<std::string>& default_files, bool dir_listing,
    unsigned int client_max_body_size, const std::string& upload_dir)
    : route_(route),
      allowed_methods_(allowed_methods),
      redirect_(InitRedirect(redirect)),
      is_cgi_(IsCgiLocation(route.first)),
      cgi_extensions_(cgi_extensions),
      alias_dir_(alias_dir),
      default_files_(default_files),
      dir_listing_(dir_listing),
      client_max_body_size_(client_max_body_size),
      upload_dir_(upload_dir)
{}

const std::pair<std::string /*path*/, bool /*is_exact_match*/>& LocationConfig::route() const
{
    return route_;
}

const std::vector<http::Method>& LocationConfig::allowed_methods() const
{
    return allowed_methods_;
}

const std::pair<int /*status_code*/, std::string /*new_route*/>& LocationConfig::redirect() const
{
    return redirect_;
}

bool LocationConfig::is_cgi() const
{
    return is_cgi_;
}

const std::vector<std::string>& LocationConfig::cgi_extensions() const
{
    return cgi_extensions_;
}

const std::string& LocationConfig::alias_dir() const
{
    return alias_dir_;
}

const std::vector<std::string>& LocationConfig::default_files() const
{
    return default_files_;
}

bool LocationConfig::dir_listing() const
{
    return dir_listing_;
}

unsigned int LocationConfig::client_max_body_size() const
{
    return client_max_body_size_;
}

const std::string& LocationConfig::upload_dir() const
{
    return upload_dir_;
}

std::pair<int /*status_code*/, std::string /*new_route*/> LocationConfig::InitRedirect(
    const std::pair<int, std::string>& value)
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
    LOG(DEBUG) << "CGI extensions: ";
    for (size_t i = 0; i < cgi_extensions_.size(); i++) {
        LOG(DEBUG) << "  " << cgi_extensions_[i];
    }
    LOG(DEBUG) << "Alias directory: " << alias_dir_;
    LOG(DEBUG) << "Default file: ";
    for (size_t i = 0; i < default_files_.size(); i++) {
        LOG(DEBUG) << "  " << default_files_[i];
    }
    LOG(DEBUG) << "Directory listing: " << dir_listing_;
    LOG(DEBUG) << "Client max body size: " << client_max_body_size_;
}

}  // namespace config
