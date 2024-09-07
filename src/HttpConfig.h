#ifndef WS_HTTPCONFIG_H
#define WS_HTTPCONFIG_H

#include "ServerConfig.h"
#include "utils/shared_ptr.h"
#include <cstddef>
#include <string>

class   HttpConfig : public IConfig {

  private:
    HttpConfig();
    static const int kDefaultKeepaliveTimeout;
    static size_t kDefaultClientMaxBodySize;
    static const std::map<int, std::string> kDefaultErrorPages;
    static const std::string kDefaultRootDir;
    static const std::string kDefaultDefaultFile;
    static const std::string kDefaultDirListing;
    typedef std::pair<std::string, std::string> Setting;
    void InitKeepaliveTimeout(const std::string& value);
    void InitClientMaxBodySize(const std::string& value);
    void InitErrorPages(const std::string& value);
    void InitRootDir(const std::string& value);
    void InitDefaultFile(const std::string& value);
    void InitDirListing(const std::string& value);
    void InitServers(const std::vector<Setting>& server_settings);

  public:
    HttpConfig(std::vector<Setting> settings);
    ~HttpConfig();
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
    typedef std::vector<ServerConfig>::const_iterator ServerConfigIt;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
};

#endif  // WS_HTTPCONFIG_H
