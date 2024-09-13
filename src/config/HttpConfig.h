#ifndef WS_HTTPCONFIG_H
#define WS_HTTPCONFIG_H

#include "ServerConfig.h"
#include <cstddef>
#include <string>

namespace config {
class   HttpConfig {

  private:
    int InitKeepaliveTimeout(int value);
    size_t  InitClientMaxBodySize(size_t value, const std::string& unit);
    std::map<int, std::string>  InitErrorPages(const std::map<int, std::string>& value);
    const std::string&  InitRootDir(const std::string& value);
    const std::string&  InitDefaultFile(const std::string& value);

  public:
    HttpConfig(int keepalive_timeout, size_t client_max_body_size, const std::map<int, std::string>& error_pages,
               const std::string& root_dir, const std::string& default_file, const std::string& dir_listing,
               const std::vector<ServerConfig>& server_configs);
    int keepalive_timeout() const;
    size_t client_max_body_size() const;
    const std::map<int, std::string>& error_pages() const;
    const std::string& root_dir() const;
    const std::string& default_file() const;
    const std::string& dir_listing() const;
    const std::vector<ServerConfig>&  server_configs() const;

  private:
    int keepalive_timeout_;
    size_t client_max_body_size_;
    std::map</* status code */ int, /* error page path */ std::string> error_pages_;
    std::vector<ServerConfig> server_configs_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
};

}  // namespace config

#endif  // WS_HTTPCONFIG_H
