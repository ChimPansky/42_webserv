#ifndef WS_HTTPCONFIG_H
#define WS_HTTPCONFIG_H

#include <cstddef>
#include <map>
#include <string>

#include "ServerConfig.h"

namespace config {
class HttpConfig {
  private:
    int InitKeepaliveTimeout(int value);
    size_t InitClientMaxBodySize(size_t value, const std::string& unit);
    std::map<int, std::string> InitErrorPages(const std::map<int, std::string>& value);

  public:
    HttpConfig(int keepalive_timeout, size_t client_max_body_size,
               const std::map<int, std::string>& error_pages,
               const std::vector<ServerConfig>& server_configs);
    int keepalive_timeout() const;
    size_t client_max_body_size() const;
    const std::map<int, std::string>& error_pages() const;
    const std::vector<ServerConfig>& server_configs() const;
    static const int kDefaultKeepaliveTimeout;
    static size_t kDefaultClientMaxBodySize;
    // default error pages?

  private:
    int keepalive_timeout_;
    size_t client_max_body_size_;
    std::map</* status code */ int, /* error page path */ std::string> error_pages_;
    std::vector<ServerConfig> server_configs_;
};

}  // namespace config

#endif  // WS_HTTPCONFIG_H
