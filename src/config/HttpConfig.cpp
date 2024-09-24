#include "HttpConfig.h"

#include "config/utils.h"

namespace config {

const int HttpConfig::kDefaultKeepaliveTimeout = 65;
const size_t HttpConfig::kDefaultClientMaxBodySize = 1048576;

HttpConfig::HttpConfig(int keepalive_timeout, size_t client_max_body_size,
                       const std::map<int, std::string>& error_pages,
                       const std::vector<ServerConfig>& server_configs)
    : keepalive_timeout_(InitKeepaliveTimeout(keepalive_timeout)),
      client_max_body_size_(InitClientMaxBodySize(client_max_body_size)),
      error_pages_(InitErrorPages(error_pages)), server_configs_(server_configs)
{}

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

const std::vector<ServerConfig>& HttpConfig::server_configs() const
{
    return server_configs_;
}

int HttpConfig::InitKeepaliveTimeout(int value)
{
    if (value < 1 || value > 120) {  // if 0, means keepalive is off
        throw std::runtime_error("Invalid configuration file: invalid keepalive_timeout value.");
    }
    return value;
}

size_t HttpConfig::InitClientMaxBodySize(size_t value)
{
    if (value < 1 || value > 1073741824) {     // minimum size is 1 byte, maximum size is 1GB
        throw std::runtime_error("Invalid configuration file: invalid client_max_body_size value.");
    }
    return value;
}

const std::map<int, std::string>& HttpConfig::InitErrorPages(
    const std::map<int, std::string>& value)
{
    typedef std::map<int, std::string>::const_iterator ErrorPagesIt;
    for (ErrorPagesIt it = value.begin(); it != value.end(); ++it) {
        if (it->first < 400 || it->first > 599) {
            throw std::runtime_error("Invalid configuration file: invalid error_page status code.");
        } else if (!config::CheckFileExtension(it->second, ".html")) {
            throw std::runtime_error("Invalid configuration file: invalid error_page path.");
        }
    }
    return value;
}

void config::HttpConfig::Print() const
{
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "--HTTP configuration: --";
    LOG(DEBUG) << "Keepalive timeout: " << keepalive_timeout_;
    LOG(DEBUG) << "Client max body size: " << client_max_body_size_;
    LOG(DEBUG) << "Error pages:";
    for (std::map<int, std::string>::const_iterator it = error_pages_.begin();
         it != error_pages_.end(); ++it) {
        LOG(DEBUG) << "  " << it->first << " -> " << it->second;
    }
    for (size_t i = 0; i < server_configs_.size(); i++) {
        server_configs_[i].Print();
    }
}
}  // namespace config
