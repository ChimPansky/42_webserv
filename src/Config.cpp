#include "Config.h"

#include <vector>

#include "ServerBlock.h"
#include "utils/unique_ptr.h"

Config::Config()
    : mx_type_("select"),
      error_log_path_("/logs/error.log"),
      error_log_level_("info"),
      keepalive_timeout_(65),
      client_max_body_size_(1048576),
      error_pages_(),
      listeners_(),
      server_configs_(),
      root_dir_(""),
      default_file_("index.html"),
      dir_listing_("")
{}

Config::Config(std::vector<setting> settings)
    : mx_type_("select"),
      error_log_path_("/logs/error.log"),
      error_log_level_("info"),
      keepalive_timeout_(65),
      client_max_body_size_(1048576),
      error_pages_(),
      listeners_(),
      server_configs_(),
      root_dir_(""),
      default_file_("index.html"),
      dir_listing_("")
{
    // InitConfig(settings);
    (void)settings;
    //  when server blocks are parsed:
    for (std::vector<utils::unique_ptr<ServerBlock> >::iterator it = server_configs_.begin();
         it != server_configs_.end(); it++) {
        listeners_.insert(listeners_.end(), (*it)->listeners().begin(), (*it)->listeners().end());
    }
}

Config::Config(const Config& copy)
    : mx_type_(copy.mx_type_),
      error_log_path_(copy.error_log_path_),
      error_log_level_(copy.error_log_level_),
      keepalive_timeout_(copy.keepalive_timeout_),
      client_max_body_size_(copy.client_max_body_size_),
      error_pages_(copy.error_pages_),
      listeners_(copy.listeners_),
      server_configs_(copy.server_configs_),
      root_dir_(copy.root_dir_),
      default_file_(copy.default_file_),
      dir_listing_(copy.dir_listing_)
{}

Config& Config::operator=(const Config& copy)
{
    if (this == &copy) {
        return *this;
    }
    mx_type_ = copy.mx_type_;
    error_log_path_ = copy.error_log_path_;
    error_log_level_ = copy.error_log_level_;
    keepalive_timeout_ = copy.keepalive_timeout_;
    client_max_body_size_ = copy.client_max_body_size_;
    error_pages_ = copy.error_pages_;
    listeners_ = copy.listeners_;
    server_configs_ = copy.server_configs_;
    root_dir_ = copy.root_dir_;
    default_file_ = copy.default_file_;
    dir_listing_ = copy.dir_listing_;
    return *this;
}

Config::~Config()
{}

const std::string& Config::mx_type() const
{
    return mx_type_;
}

const std::string& Config::error_log_path() const
{
    return error_log_path_;
}

const std::string& Config::error_log_level() const
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

size_t Config::client_max_body_size() const
{
    return client_max_body_size_;
}

const std::map<int, std::string>& Config::error_pages() const
{
    return error_pages_;
}

const std::string& Config::root_dir()
{
    return root_dir_;
}

const std::string& Config::default_file() const
{
    return default_file_;
}

const std::string& Config::dir_listing() const
{
    return dir_listing_;
}

const std::vector<utils::unique_ptr<ServerBlock> >& Config::server_configs() const
{
    return server_configs_;
}

/* utils::unique_ptr<ServerBlock> Config::FindServerSettings(std::pair<in_addr_t, in_port_t>
listener)
{
    for (std::vector<utils::unique_ptr<ServerBlock> >::iterator it = server_configs_.begin();
         it != server_configs_.end(); it++) {
        if (std::find((*it)->listeners().begin(), (*it)->listeners().end(), listener) !=
            (*it)->listeners().end()) {
            return *it;
        }
    }
    return *server_configs_.end();
} */

const std::vector<std::string> Config::GetMainTokens()
{
    std::vector<std::string> tokens;

    tokens.push_back("error_log");
    tokens.push_back("use");

    return tokens;
}

const std::vector<std::string> Config::GetHttpTokens()
{
    std::vector<std::string> tokens;

    tokens.push_back("keepalive_timeout");
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");
    tokens.push_back("client_max_body_size");
    tokens.push_back("error_page");

    return tokens;
}
