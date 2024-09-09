#include "ServerConfig.h"

ServerConfig::ServerConfig(std::vector<Setting> settings, const std::vector<utils::shared_ptr<LocationConfig> >& location_configs)
    //: locations_(location_configs)
{
    // init server settings just like in Config
    (void)settings;
    (void)location_configs;
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

const std::map<std::string, utils::shared_ptr<LocationConfig> >& ServerConfig::locations()
{
    return locations_;
}

bool    ServerConfig::IsValid() const
{
    //  Check if all attributes are initialized
    return true;
}
