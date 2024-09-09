#include "LocationConfig.h"

LocationConfig::LocationConfig(const std::vector<Setting>& settings, const std::string& lvl_descrt)
{
    // parse values from the map
    (void)settings;
    (void)lvl_descrt;
}

const std::string& LocationConfig::route() const
{
    return route_;
}

const std::vector<std::string>& LocationConfig::allowed_methods() const
{
    return allowed_methods_;
}

const std::pair<int, std::string>& LocationConfig::redirect() const
{
    return redirect_;
}

bool LocationConfig::is_cgi() const
{
    return is_cgi_;
}

const std::vector<std::string>& LocationConfig::cgi_paths() const
{
    return cgi_paths_;
}

const std::vector<std::string>& LocationConfig::cgi_extensions() const
{
    return cgi_extensions_;
}

const std::string& LocationConfig::root_dir()
{
    return root_dir_;
}

const std::string& LocationConfig::default_file() const
{
    return default_file_;
}

const std::string& LocationConfig::dir_listing() const
{
    return dir_listing_;
}
