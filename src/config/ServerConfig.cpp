#include "ServerConfig.h"

ServerConfig::ServerConfig(const std::string& access_log_path, const std::string& access_log_level, const std::string& error_log_path,
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners, const std::string& root_dir,
    const std::string& default_file, const std::string& dir_listing, const std::vector<std::string>& server_names,
    const std::map<std::string, LocationConfig>& locations)
  : access_log_path_(access_log_path),
    access_log_level_(access_log_level),
    error_log_path_(error_log_path),
    listeners_(listeners),
    root_dir_(root_dir),
    default_file_(default_file),
    dir_listing_(dir_listing),
    server_names_(server_names),
    locations_(locations)
{
}

const std::string& ServerConfig::access_log_path() const
{
    return access_log_path_;
}

const std::string& ServerConfig::access_log_level() const
{
    return access_log_level_;
}

const std::string& ServerConfig::error_log_path() const
{
    return error_log_path_;
}

const std::vector<std::pair<in_addr_t, in_port_t> >& ServerConfig::listeners() const
{
    return listeners_;
}

const std::string& ServerConfig::root_dir()
{
    return root_dir_;
}

const std::string& ServerConfig::default_file()
{
    return this->default_file_;
}

const std::string& ServerConfig::dir_listing()
{
    return dir_listing_;
}

const std::vector<std::string>& ServerConfig::server_names()
{
    return server_names_;
}

const std::map<std::string, LocationConfig>& ServerConfig::locations()
{
    return locations_;
}
