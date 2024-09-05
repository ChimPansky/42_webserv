#include "Config.h"

Config::Config(std::vector<S> settings)
    : mx_type_("select"),
      error_log_path_("/logs/error.log"),
      error_log_level_("info"),
      keepalive_timeout_(65),
      client_max_body_size_(1048576),  // 1MB
      root_dir_("/var"),
      default_file_("index.html"),
      dir_listing_(false)
{
    error_pages_[404] = "/errors/404.html";
    this->InitConfig(settings);
    // temp default listeners before config parsing is ready
    listeners_.push_back(std::make_pair(c_api::IPv4FromString("localhost"), in_port_t(8081)));
    listeners_.push_back(std::make_pair(c_api::IPv4FromString("localhost"), in_port_t(8082)));

    // InitListeners();
}

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

bool Config::dir_listing() const
{
    return dir_listing_;
}

const std::vector<utils::unique_ptr<ServerBlock> >& Config::server_configs() const
{
    return server_configs_;
}

void Config::InitConfig(std::vector<S> settings)
{
    std::map<std::string, FunctionPointer> setters = InitSettings();

    for (std::vector<S>::iterator it = settings.begin(); it != settings.end(); it++) {
        if (it->first == "server") {
            InitServers(ServerBlock::ExtractBlock(it));
            continue;
        }
        MethodsIt method = setters.find(it->first);
        if (method != setters.end()) {
            (this->*(method->second))(it->second);
        } else if (it->first != "http" && it->first != "}") {
            throw std::runtime_error("Invalid configuration file: invalid directive: " + it->first);
        }
    }
}

void Config::InitServers(const std::vector<S>& server_settings)
{
    ServerBlock* server = new ServerBlock(server_settings);
    server_configs_.push_back(utils::unique_ptr<ServerBlock>(server));
}

void Config::InitMxType(const std::string& value)
{
    if (value == "select" || value == "poll" || value == "epoll") {
        mx_type_ = value;
    } else {
        throw std::runtime_error("Invalid configuration file: invalid mx_type: " + value);
    }
}

void Config::InitErrorLog(const std::string& value)
{
    (void)value;
    //  TODO
}

void Config::InitKeepaliveTimeout(const std::string& value)
{
    (void)value;
    //  TODO
}

void Config::InitClientMaxBodySize(const std::string& value)
{
    (void)value;
    //  TODO
}

void Config::InitErrorPages(const std::string& value)
{
    (void)value;
    //  TODO
}

void Config::InitRootDir(const std::string& value)
{
    (void)value;
    //  TODO
}

void Config::InitDefaultFile(const std::string& value)
{
    (void)value;
    //  TODO
}

void Config::InitDirListing(const std::string& value)
{
    (void)value;
    //  TODO
}

void Config::InitListeners()
{
    //  when server blocks are parsed:
    if (server_configs_.empty()) {
        throw std::runtime_error("Invalid configuration file: no server blocks.");
    }
    for (std::vector<utils::unique_ptr<ServerBlock> >::iterator it = server_configs_.begin();
         it != server_configs_.end(); it++) {
        listeners_.insert(listeners_.end(), (*it)->listeners().begin(), (*it)->listeners().end());
    }
}

std::map<std::string, Config::FunctionPointer> Config::InitSettings()
{
    std::map<std::string, FunctionPointer> methods;

    methods["use"] = &Config::InitMxType;
    methods["error_log"] = &Config::InitErrorLog;
    methods["keepalive_timeout"] = &Config::InitKeepaliveTimeout;
    methods["client_max_body_size"] = &Config::InitClientMaxBodySize;
    methods["error_page"] = &Config::InitErrorPages;
    methods["root"] = &Config::InitRootDir;
    methods["index"] = &Config::InitDefaultFile;
    methods["autoindex"] = &Config::InitDirListing;

    return methods;
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
