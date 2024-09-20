#include "LocationConfig.h"
#include <iostream>

namespace config {

const std::string LocationConfig::kDefaultRedirectPath = "/new_location";
const int LocationConfig::kDefaultRedirectCode = 301;

LocationConfig::LocationConfig(const std::string& route, const std::vector<std::string>& allowed_methods,
    const std::pair<int, std::string>& redirect, const std::vector<std::string>& cgi_paths, const std::vector<std::string>& cgi_extensions,
    const std::string& root_dir, const std::string& default_file, const std::string& dir_listing)
  : route_(InitRoute(route)),
    allowed_methods_(allowed_methods),
    redirect_(InitRedirect(redirect)),
    is_cgi_(!cgi_paths.empty() || !cgi_extensions.empty()),
    cgi_paths_(InitCgiPaths(cgi_paths)),
    cgi_extensions_(InitCgiExtensions(cgi_extensions)),
    root_dir_(InitRootDir(root_dir)),
    default_file_(InitDefaultFile(default_file)),
    dir_listing_(dir_listing)
{
}

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

const std::vector<std::string>&    LocationConfig::cgi_paths() const
{
    return cgi_paths_;
}

const std::vector<std::string>&    LocationConfig::cgi_extensions() const
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

const std::string&  LocationConfig::InitRoute(const std::string& value)
{
    //To Do
    return value;
}

std::pair<int, std::string>  LocationConfig::InitRedirect(const std::pair<int, std::string>& value)
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

const std::string&  LocationConfig::InitRootDir(const std::string& value)
{
    if (access(value.c_str(), F_OK | R_OK | X_OK) == -1) {
        throw std::runtime_error("Invalid configuration file: invalid path to the root directory");
    }
    return value;
}

const std::string&  LocationConfig::InitDefaultFile(const std::string& value)
{
   if (access(value.c_str(), F_OK | R_OK | W_OK) == -1) {
        throw std::runtime_error("Invalid configuration file: invalid path to the index file");
    }
    return value;
}

}  // namespace config
