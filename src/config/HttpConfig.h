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
    HttpConfig(unsigned int keepalive_timeout, const std::map<int, std::string>& error_pages,
               const std::vector<ServerConfig>& server_configs);

    unsigned int keepalive_timeout() const;
    const std::map<int, std::string>& error_pages() const;
    const std::vector<ServerConfig>& server_configs() const;

    static inline unsigned int kDefaultKeepaliveTimeout() { return 65; }
    void Print() const;

  private:
    unsigned int keepalive_timeout_;
    std::map</* status code */ int, /* error page path */ std::string> error_pages_;
    std::vector<ServerConfig> server_configs_;
};

}  // namespace config

#endif  // WS_HTTPCONFIG_H
