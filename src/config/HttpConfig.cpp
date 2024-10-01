#include "HttpConfig.h"

#include "config/utils.h"

namespace config {

HttpConfig::HttpConfig(int keepalive_timeout, size_t client_max_body_size,
                       const std::map<int, std::string>& error_pages,
                       const std::vector<ServerConfig>& server_configs)
    : keepalive_timeout_(keepalive_timeout), client_max_body_size_(client_max_body_size),
      error_pages_(error_pages), server_configs_(server_configs)
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
