#include "Config.h"
#include "ServerBlock.h"

Config::Config(const char* config_path)
  : _config_file(),
    _use("select"),    
    _http_block(),
    _servers()
{
    std::string path(config_path);

    if (path.substr(path.find_last_of('.') + 1) != "conf") {
        throw std::runtime_error("Invalid configuration file suffix");
    }

    _config_file.open(config_path);
    if (!_config_file.is_open()) {
        throw std::runtime_error("Cannot open configuration file");
    }
}

std::ifstream& Config::config_file()
{
    return _config_file;
}

const std::string& Config::use() const
{
    return _use;
}

const std::string&  Config::error_log() const
{
    return _error_log;
}

const std::string&  Config::error_log_level() const
{
    return _error_log_level;
}

const HttpBlock& Config::http_block()
{
    return _http_block;
}

const std::vector<ServerBlock>& Config::servers() const
{
    return _servers;
}

void    Config::processFile()
{
    std::string content;
    
    while ( std::getline(_config_file, content) ) {
       
    }
    if (content.empty()) {
        throw std::runtime_error("Empty configuration file.\n");
    }
    _config_file.close();
}
