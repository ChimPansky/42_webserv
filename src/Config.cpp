#include "Config.h"
#include "conf_pars.h"
#include <iostream>
#include <vector>
#include "ServerBlock.h"
#include "utils/unique_ptr.h"

Config::Config(const char* config_path)
  : mx_type_("select"),
    error_log_path_("/logs/error.log"),
    error_log_level_("info"),
    keepalive_timeout_(65),
    client_max_body_size_(1048576),
    listeners_(),
    server_configs_()
{
    std::string path(config_path);
    std::ifstream _config_file;

    if (path.length() < 6 || path.substr(path.find_last_of('.') + 1) != "conf") {
        std::cerr << "Invalid configuration file extension." << std::endl;
        return ;
    }

    _config_file.open(config_path);
    if (!_config_file.is_open()) {
        std::cerr << "Failed to open configuration file." << std::endl;
        return ;
    }
    //config_pars::ProcessFile(_config_file);

    // when server blocks are parsed:
    for (std::vector<utils::unique_ptr<ServerBlock> >::iterator it = server_configs_.begin(); it != server_configs_.end(); it++) {
        listeners_.push_back((*it)->listener());
    }
    _config_file.close();
}

const std::string& Config::mx_type() const
{
    return mx_type_;
}

const std::string&  Config::error_log_path()
{
    return error_log_path_;
}

const std::string&  Config::error_log_level() const
{
    return error_log_level_;
}

const std::vector<std::pair<in_addr_t, in_port_t> > Config::listeners() const
{
    return listeners_;
}

int Config::keepalive_timeout() const
{
    return keepalive_timeout_;
}

size_t  Config::client_max_body_size() const
{
    return client_max_body_size_;
}

const std::map<int, std::string>& Config::error_pages() const
{
    return error_pages_;
}

const std::vector<utils::unique_ptr<ServerBlock> >& Config::server_configs() const
{
    return server_configs_;
}

utils::unique_ptr<ServerBlock>  Config::FindServerBlock(std::pair<in_addr_t, in_port_t> listener, const std::string& server_name)
{
    if (!server_name.empty()) {
        for (std::vector<utils::unique_ptr<ServerBlock> >::iterator it = server_configs_.begin(); it != server_configs_.end(); it++) {

            std::vector<std::string>::const_iterator iter = std::find((*it)->server_names().begin(), (*it)->server_names().end(), server_name);
            if (iter != (*it)->server_names().end()) {
                if ((*it)->listener() == listener) {
                    return *it;
                }
            }
        }
    }
    for (std::vector<utils::unique_ptr<ServerBlock> >::iterator it = server_configs_.begin(); it != server_configs_.end(); it++) {
        if ((*it)->listener() == listener) {
            return *it;
        }
    }
    return *server_configs_.begin();
}

const std::vector<std::string>    Config::GetTokens() {
    
    std::vector<std::string> tokens;

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

