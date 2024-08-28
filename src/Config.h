#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include "HttpBlock.h"
#include <fstream>
#include <string>
#include <vector>

class ServerBlock;
class LocationBlock;

// hopefully will be trivially-copyable
class Config {

  private:
    Config();
    std::ifstream _config_file;
    std::string _use;
    std::string _error_log;
    std::string _error_log_level;
    HttpBlock  _http_block;
    std::vector<ServerBlock> _servers;

  public:
    Config(const char* config_path);
    std::ifstream& config_file();
    const std::string& use() const;
    const std::string&  error_log() const;
    const std::string&  error_log_level() const;
    const std::string&  access_log() const;
    const std::string&  access_log_level() const;
    const HttpBlock& http_block();
    const std::vector<ServerBlock>& servers() const;
    void  processFile();
};

#endif  // WS_CONFIG_H
