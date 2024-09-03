#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include <netinet/in.h>
#include <utils/unique_ptr.h>

#include <map>
#include <string>
#include <vector>

#include "ServerBlock.h"

typedef std::pair<std::string, std::string> setting;

// hopefully will be trivially-copyable
class Config {
  public:
    Config();
    Config(std::vector<setting> settings);
    Config(const Config& other);
    Config& operator=(const Config& other);
    ~Config();
    const std::string& mx_type() const;
    const std::string& error_log_path() const;
    const std::string& error_log_level() const;
    int keepalive_timeout() const;
    size_t client_max_body_size() const;
    const std::map<int, std::string>& error_pages() const;
    const std::string& root_dir();
    const std::string& default_file() const;
    const std::string& dir_listing() const;
    const std::vector<std::pair<in_addr_t, in_port_t> > listeners() const;
    const std::vector<utils::unique_ptr<ServerBlock> >& server_configs() const;
    // utils::unique_ptr<ServerBlock> FindServerSettings(std::pair<in_addr_t, in_port_t>);
    static const std::vector<std::string> GetMainTokens();
    static const std::vector<std::string> GetHttpTokens();
    void InitConfig(std::vector<setting> settings);
    void InitServers() const;

  private:
    std::string mx_type_;
    std::string error_log_path_;
    std::string error_log_level_;
    int keepalive_timeout_;
    size_t client_max_body_size_;
    std::map</* status code */ int, /* error page path */ std::string> error_pages_;
    std::vector<std::pair<in_addr_t, in_port_t> > listeners_;
    std::vector<utils::unique_ptr<ServerBlock> > server_configs_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
};

#endif  // WS_CONFIG_H
