#include "HttpConfig.h"

namespace config {

const int HttpConfig::kDefaultKeepaliveTimeout = 65;
size_t HttpConfig::kDefaultClientMaxBodySize = 1048576;
const std::string HttpConfig::kDefaultDefaultFile = "index.html";
const std::string HttpConfig::kDefaultDirListing = "off";

HttpConfig::HttpConfig(int keepalive_timeout, size_t client_max_body_size, const std::map<int, std::string>& error_pages,
            const std::string& root_dir, const std::string& default_file, const std::string& dir_listing,
            const std::vector<ServerConfig>& server_configs)
  : keepalive_timeout_(InitKeepaliveTimeout(keepalive_timeout)),
    client_max_body_size_(InitClientMaxBodySize(client_max_body_size, "MB")),
    error_pages_(InitErrorPages(error_pages)),
    server_configs_(server_configs),
    root_dir_(InitRootDir(root_dir)),
    default_file_(InitDefaultFile(default_file)),
    dir_listing_(dir_listing)
{
}

int HttpConfig::keepalive_timeout() const
{
    return keepalive_timeout_;
}

size_t HttpConfig::client_max_body_size() const
{
    return client_max_body_size_;
}

const std::map<int, std::string>& HttpConfig::error_pages() const
{
    return error_pages_;
}

const std::string& HttpConfig::root_dir() const
{
    return root_dir_;
}

const std::string& HttpConfig::default_file() const
{
    return default_file_;
}

const std::string& HttpConfig::dir_listing() const
{
    return dir_listing_;
}

const std::vector<ServerConfig>& HttpConfig::server_configs() const
{
    return server_configs_;
}

int HttpConfig::InitKeepaliveTimeout(int value)
{
    if (value < 1 || value > 120) { // if 0, means keepalive is off
        throw std::runtime_error("Invalid configuration file: invalid keepalive_timeout value.");
    }
    return value;
}

size_t  HttpConfig::InitClientMaxBodySize(size_t value, const std::string& unit)
{
    (void)unit;
    //  TODO

    // Setting size to 0 disables checking of client request body size.
    // minimum size is 1 byte, maximum size is 1GB
    return value;
}

std::map<int, std::string>    HttpConfig::InitErrorPages(const std::map<int, std::string>& value)
{
    (void)value;
    //  TODO
    return std::map<int, std::string>();
}

const std::string&  HttpConfig::InitRootDir(const std::string& value)
{
    if (access(value.c_str(), F_OK | R_OK | X_OK) == -1) {
        throw std::runtime_error("Invalid configuration file: invalid path to the root directory");
    }
    return value;
}

const std::string&  HttpConfig::InitDefaultFile(const std::string& value)
{
    if (access(value.c_str(), F_OK | R_OK | W_OK) == -1) {
        throw std::runtime_error("Invalid configuration file: invalid path to the index file");
    }
    return value;
}

}  // namespace config
