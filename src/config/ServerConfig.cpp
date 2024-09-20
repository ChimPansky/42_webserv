#include "ServerConfig.h"

namespace config {

const Severity ServerConfig::kDefaultAccessLogLevel = INFO;
const std::string ServerConfig::kDefaultAccessLogPath = "/log/access.log";
const std::string ServerConfig::kDefaultErrorLogPath = "/log/error.log";

ServerConfig::ServerConfig(const std::string& access_log_path, Severity access_log_level, const std::string& error_log_path,
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners, const std::string& root_dir,
    const std::string& default_file, const std::string& dir_listing, const std::vector<std::string>& server_names,
    const std::vector<LocationConfig>& locations)
  : access_log_path_(InitAccessLog(access_log_path)),
    access_log_level_(access_log_level),
    error_log_path_(InitErrorLogPath(error_log_path)),
    listeners_(listeners),
    root_dir_(InitRootDir(root_dir)),
    default_file_(InitDefaultFile(default_file)),
    dir_listing_(dir_listing),
    server_names_(InitServerNames(server_names)),
    locations_(locations)
{
}

const std::string& ServerConfig::access_log_path() const
{
    return access_log_path_;
}

Severity    ServerConfig::access_log_level() const
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

const std::vector<LocationConfig>& ServerConfig::locations()
{
    return locations_;
}

const std::string&  ServerConfig::InitAccessLog(const std::string& value)
{
    if (value.length() < 5 || (value.find_last_of('.') != std::string::npos && value.substr(value.find_last_of('.')) != ".log")) {
        throw std::runtime_error("Invalid log file suffix.");
    } else if (access(value.c_str(), F_OK | R_OK | W_OK) == -1) {
        throw std::runtime_error("Invalid configuration file: invalid path to the error log file" + value);
    }
    return value;
}

const std::string&  ServerConfig::InitErrorLogPath(const std::string& value)
{
    if (value.length() < 5 || (value.find_last_of('.') != std::string::npos && value.substr(value.find_last_of('.')) != ".log")) {
        throw std::runtime_error("Invalid log file suffix.");
    } else if (access(value.c_str(), F_OK | R_OK | W_OK) == -1) {
        throw std::runtime_error("Invalid configuration file: invalid path to the error log file" + value);
    }
    return value;
}

std::vector<std::pair<in_addr_t, in_port_t> > InitListeners(const std::string& value)
{
    (void)value;
    //TO DO
    return std::vector<std::pair<in_addr_t, in_port_t> >();
}

const std::string&  ServerConfig::InitRootDir(const std::string& value)
{
    if (access(value.c_str(), F_OK | R_OK | X_OK) == -1) {
        throw std::runtime_error("Invalid configuration file: invalid path to the root directory");
    }
    return value;
}

const std::string&  ServerConfig::InitDefaultFile(const std::string& value)
{
    if (access(value.c_str(), F_OK | R_OK | W_OK) == -1) {
        throw std::runtime_error("Invalid configuration file: invalid path to the index file");
    }
    return value;
}

std::vector<std::string> ServerConfig::InitServerNames(const std::vector<std::string>& value)
{
    (void)value;
    //TO DO
    return std::vector<std::string>();
}

}  // namespace config
