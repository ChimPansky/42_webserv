#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include <c_api/utils.h>
#include <netinet/in.h>
#include <utils/unique_ptr.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "ServerBlock.h"

typedef std::vector<utils::unique_ptr<ServerBlock> >::const_iterator ServerBlockIt;

// hopefully will be trivially-copyable
class Config {
  private:
    typedef void (Config::*FunctionPointer)(const std::string&);
    typedef std::map<std::string, FunctionPointer>::const_iterator MethodsIt;
    std::map<std::string, FunctionPointer> InitSettings();

  public:
    Config();
    const std::string& mx_type() const;
    const std::string& error_log_path() const;
    const std::string& error_log_level() const;
    int keepalive_timeout() const;
    size_t client_max_body_size() const;
    const std::map<int, std::string>& error_pages() const;
    const std::string& root_dir();
    const std::string& default_file() const;
    bool dir_listing() const;
    const std::vector<std::pair<in_addr_t, in_port_t> > listeners() const;
    const std::vector<utils::unique_ptr<ServerBlock> >& server_configs() const;
    // utils::unique_ptr<ServerBlock> FindServerSettings(std::pair<in_addr_t, in_port_t>);
    void InitConfig(std::vector<setting> settings);
    void InitServers(const std::vector<setting>& server_settings);
    void InitMxType(const std::string& value);
    void InitErrorLog(const std::string& value);
    void InitKeepaliveTimeout(const std::string& value);
    void InitClientMaxBodySize(const std::string& value);
    void InitErrorPages(const std::string& value);
    void InitRootDir(const std::string& value);
    void InitDefaultFile(const std::string& value);
    void InitDirListing(const std::string& value);
    void InitListeners();
    static const std::vector<std::string> GetMainTokens();
    static const std::vector<std::string> GetHttpTokens();

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
