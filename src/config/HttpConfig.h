#ifndef WS_HTTPCONFIG_H
#define WS_HTTPCONFIG_H

#include "ServerConfig.h"
#include <cstddef>
#include <string>

class   HttpConfig {

  private:
    static const int kDefaultKeepaliveTimeout;
    static size_t kDefaultClientMaxBodySize;
    static const std::map<int, std::string> kDefaultErrorPages; // ?
    static const std::string kDefaultRootDir;
    static const std::string kDefaultDefaultFile;
    static const std::string kDefaultDirListing;
    typedef std::pair<std::string, std::string> Setting;
    int InitKeepaliveTimeout(const std::string& value);
    size_t  InitClientMaxBodySize(const std::string& value);
    const std::map<int, std::string>  InitErrorPages(const std::string& value);
    const std::string&  InitRootDir(const std::string& value);
    const std::string&  InitDefaultFile(const std::string& value);
    const std::string&  InitDirListing(const std::string& value);
    const std::vector<ServerConfig>&  InitServers(const std::vector<Setting>& server_settings);

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

#endif  // WS_HTTPCONFIG_H
