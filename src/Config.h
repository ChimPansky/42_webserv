#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include <fstream>

#include "ServerBlock.h"

#include <string>
#include <vector>

// hopefully will be trivially-copyable
class Config {

  private:
    Config();
    std::string mx_type_;
    std::string error_log_;
    std::string error_log_level_;
    int   keepalive_timeout_;
    std::vector<ServerBlock> servers_;

  public:
    Config(const char* config_path);
    const std::string&  mx_type() const;
    const std::string&  error_log() const;
    const std::string&  error_log_level() const;
    int   keepalive_timeout() const;
    const std::vector<ServerBlock>& servers() const;
    void  processFile(std::ifstream&);
    static const std::vector<std::string>    GetTokens();
    void  set_mx_type(const std::string& mx_type);
    void  set_error_log(const std::string& error_log);
    void  set_error_log_level(const std::string& error_log_level);
    void  set_keepalive_timeout(int keepalive_timeout);
    void  set_servers(const std::vector<ServerBlock>& servers);
};

#endif  // WS_CONFIG_H
