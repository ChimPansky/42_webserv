#include "LocationConfig.h"

namespace config {

const std::string LocationConfig::kDefaultRedirectPath = "/new_location";
const int LocationConfig::kDefaultRedirectCode = 301;
const std::string LocationConfig::kDefaultRootDir = "/docs";
const std::string LocationConfig::kDefaultIndexFile = "index.html";
const std::string LocationConfig::kDefaultDirListing = "off";

LocationConfig::LocationConfig(const std::string& route,
                               const std::vector<std::string>& allowed_methods,
                               const std::pair<int, std::string>& redirect,
                               const std::vector<std::string>& cgi_paths,
                               const std::vector<std::string>& cgi_extensions,
                               const std::string& root_dir, const std::string& default_file,
                               const std::string& dir_listing)
    : route_(InitRoute(route)), allowed_methods_(allowed_methods),
      redirect_(InitRedirect(redirect)), is_cgi_(!cgi_paths.empty() || !cgi_extensions.empty()),
      cgi_paths_(InitCgiPaths(cgi_paths)), cgi_extensions_(InitCgiExtensions(cgi_extensions)),
      root_dir_(root_dir), default_file_(default_file), dir_listing_(dir_listing)
{}

const std::string& LocationConfig::route() const
{
    return route_;
}

const std::vector<std::string>& LocationConfig::allowed_methods() const
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

const std::string& LocationConfig::root_dir()
{
    return root_dir_;
}

const std::string& LocationConfig::default_file() const
{
    return default_file_;
}

const std::string& LocationConfig::dir_listing() const
{
    return dir_listing_;
}

const std::string& LocationConfig::InitRoute(const std::string& value)
{
    // To Do
    return value;
}

std::pair<int, std::string> LocationConfig::InitRedirect(const std::pair<int, std::string>& value)
{
    return value;
}

std::vector<std::string> LocationConfig::InitCgiPaths(const std::vector<std::string>& value)
{
    (void)value;
    return std::vector<std::string>();
}

std::vector<std::string> LocationConfig::InitCgiExtensions(const std::vector<std::string>& value)
{
    (void)value;
    return std::vector<std::string>();
}

void LocationConfig::Print() const
{
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "--Location configuration: --";
    LOG(DEBUG) << "Route: " << route_;
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
    LOG(DEBUG) << "Default file: " << default_file_;
    LOG(DEBUG) << "Directory listing: " << dir_listing_;
}

}  // namespace config
