#ifndef WS_HTTPCONFIG_H
#define WS_HTTPCONFIG_H

#include <map>
#include <string>

#include "ServerConfig.h"

namespace config {
class HttpConfig {
  private:
    const std::map<int, std::string>& InitErrorPages(const std::map<int, std::string>& value);

  public:
    HttpConfig(unsigned int keepalive_timeout, unsigned int client_max_body_size,
               const std::map<int, std::string>& error_pages,
               const std::vector<ServerConfig>& server_configs);

    unsigned int keepalive_timeout() const;
    unsigned int client_max_body_size() const;
    const std::map<int, std::string>& error_pages() const;
    const std::vector<ServerConfig>& server_configs() const;

    static inline unsigned int kDefaultKeepaliveTimeout() { return 65; }
    static inline unsigned int kDefaultClientMaxBodySize() { return 2ul << 20; }

    void Print() const;

  private:
    unsigned int keepalive_timeout_;
    unsigned int client_max_body_size_;
    std::map</* status code */ int, /* error page path */ std::string> error_pages_;
    std::vector<ServerConfig> server_configs_;
};

}  // namespace config

#endif  // WS_HTTPCONFIG_H
