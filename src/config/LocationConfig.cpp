#include "LocationConfig.h"
#include "utils.h"
#include <iostream>

namespace config {

const std::string LocationConfig::kDefaultRedirectPath = "/new_location";
const int LocationConfig::kDefaultRedirectCode = 301;
const std::string LocationConfig::kDefaultRootDir = "/docs";
const std::string LocationConfig::kDefaultIndexFile = "index.html";
const std::string LocationConfig::kDefaultDirListing = "off";

LocationConfig::LocationConfig(const std::pair<std::string, std::string>& route,
                               const std::vector<std::string>& allowed_methods,
                               const std::pair<int, std::string>& redirect,
                               const std::vector<std::string>& cgi_paths,
                               const std::vector<std::string>& cgi_extensions,
                               const std::string& root_dir, const std::string& default_file,
                               const std::string& dir_listing)
    : route_(InitRoute(route)), allowed_methods_(allowed_methods),
      redirect_(InitRedirect(redirect)), is_cgi_(route.first == "/cgi-bin/"),
      cgi_paths_(cgi_paths), cgi_extensions_(cgi_extensions),
      root_dir_(root_dir), default_file_(InitDefaultFile(default_file)), dir_listing_(dir_listing)
{}

const std::pair<std::string, LocationConfig::LocationPriority>& LocationConfig::route() const
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

const std::pair<std::string, LocationConfig::LocationPriority> LocationConfig::InitRoute(const std::pair<std::string, std::string>& value)
{
    LocationPriority priority;
    if (value.second.empty()) {
        priority = P2;
    } else if (value.second == "^~") {
        priority = P1;
    } else if (value.second == "=") {
        priority = P0;
    } else {
        throw std::runtime_error("Invalid location priority.");
    }
    return std::make_pair(value.first, priority);
}

const std::string&  LocationConfig::InitDefaultFile(const std::string& value)
{
    if (!CheckFileExtension(value, ".html")) {
        throw std::runtime_error("Invalid configuration file: invalid index file suffix.");
    }
    return value;
}

std::pair<int, std::string> LocationConfig::InitRedirect(const std::pair<int, std::string>& value)
{
    if (value.first < 300 || value.first > 399) {
        throw std::runtime_error("Invalid configuration file: invalid redirect status code.");
    } else if (!IsDirectory(value.second) && !CheckFileExtension(value.second, ".html")) { // check if it is a file or directory
        std::cout << IsDirectory(value.second) << std::endl;
        throw std::runtime_error("Invalid configuration file: invalid redirect path: " + value.second);
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
    LOG(DEBUG) << "Default file: " << default_file_;
    LOG(DEBUG) << "Directory listing: " << dir_listing_;
}

}  // namespace config
