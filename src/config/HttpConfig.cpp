#include "HttpConfig.h"

namespace config {

HttpConfig::HttpConfig(unsigned int keepalive_timeout, unsigned int client_max_body_size,
                       const std::map<int, std::string>& error_pages,
                       const std::vector<ServerConfig>& server_configs)
    : keepalive_timeout_(keepalive_timeout), client_max_body_size_(client_max_body_size),
      error_pages_(InitErrorPages(error_pages)), server_configs_(server_configs)
{}

unsigned int HttpConfig::keepalive_timeout() const
{
    return keepalive_timeout_;
}

unsigned int HttpConfig::client_max_body_size() const
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

const std::map<int, std::string>& HttpConfig::InitErrorPages(
    const std::map<int, std::string>& value)
{
    typedef std::map<int, std::string>::const_iterator ErrorPagesIt;
    for (ErrorPagesIt it = value.begin(); it != value.end(); ++it) {
        if (it->first < 400 || it->first > 599) {
            throw std::runtime_error("Invalid configuration file: invalid error_page status code.");
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
