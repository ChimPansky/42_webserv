#ifndef WS_HTTPCONFIG_H
#define WS_HTTPCONFIG_H

#include <cstddef>
#include <map>
#include <string>

#include "ServerConfig.h"

namespace config {
class HttpConfig {
  private:
    const std::map<int, std::string>& InitErrorPages(const std::map<int, std::string>& value);

  public:
    HttpConfig(size_t keepalive_timeout, size_t client_max_body_size,
               const std::map<int, std::string>& error_pages,
               const std::vector<ServerConfig>& server_configs);
    size_t keepalive_timeout() const;
    size_t client_max_body_size() const;
    const std::map<int, std::string>& error_pages() const;
    const std::vector<ServerConfig>& server_configs() const;
    static inline size_t kDefaultKeepaliveTimeout() { return 65; }
    static inline size_t kDefaultClientMaxBodySize() { return 1048576; }

    void Print() const;

  private:
    size_t keepalive_timeout_;
    size_t client_max_body_size_;
    std::map</* status code */ int, /* error page path */ std::string> error_pages_;
    std::vector<ServerConfig> server_configs_;
};

}  // namespace config

#endif  // WS_HTTPCONFIG_H
