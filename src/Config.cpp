#include "Config.h"
#include <fstream>
#include "ServerBlock.h"

Config::Config(const char* config_path)
  : mx_type_("select"),
    error_log_(""),
    error_log_level_("info"),
    keepalive_timeout_(65),
    servers_()
{
    std::string path(config_path);
    std::ifstream _config_file;

    if (path.substr(path.find_last_of('.') + 1) != "conf") {
        throw std::runtime_error("Invalid configuration file suffix");
    }

    _config_file.open(config_path);
    if (!_config_file.is_open()) {
        throw std::runtime_error("Cannot open configuration file");
    }
    //read and parse the configuration file
    _config_file.close();
}

const std::string& Config::mx_type() const
{
    return mx_type_;
}

const std::string&  Config::error_log() const
{
    return error_log_;
}

const std::string&  Config::error_log_level() const
{
    return error_log_level_;
}

int Config::keepalive_timeout() const
{
    return keepalive_timeout_;
}

const std::vector<ServerBlock>& Config::servers() const
{
    return servers_;
}

const std::vector<std::string>    Config::GetTokens() {
    
    std::vector<std::string> tokens;

    tokens.push_back("http");
    tokens.push_back("server");
    tokens.push_back("error_log");
    tokens.push_back("use");
    tokens.push_back("keepalive_timeout");
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");
    tokens.push_back("client_max_body_size");
    tokens.push_back("error_page");

    return tokens;
}

void    Config::processFile(std::ifstream& _config_file)
{
    std::string content;
    
    while ( std::getline(_config_file, content) ) {
       
    }
    if (content.empty()) {
        throw std::runtime_error("Empty configuration file.\n");
    }
    _config_file.close();
}


void  Config::set_mx_type(const std::string& mx_type)
{
     mx_type_ = mx_type;
}

void  Config::set_error_log(const std::string& error_log)
{
    error_log_ = error_log;
}

void  Config::set_error_log_level(const std::string& error_log_level)
{
    error_log_level_ = error_log_level;
}

void  Config::set_keepalive_timeout(int keepalive_timeout)
{
    keepalive_timeout_ = keepalive_timeout;
}

void  Config::set_servers(const std::vector<ServerBlock>& servers)
{
    servers_ = servers;
}
