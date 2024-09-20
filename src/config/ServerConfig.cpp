#include "ServerConfig.h"

#include "utils.h"

namespace config {

const Severity ServerConfig::kDefaultAccessLogLevel = INFO;
const std::string ServerConfig::kDefaultAccessLogPath = "/log/access.log";

ServerConfig::ServerConfig(const std::string& access_log_path, Severity access_log_level,
                           const std::string& error_log_path,
                           const std::vector<std::pair<in_addr_t, in_port_t> >& listeners,
                           const std::vector<std::string>& server_names,
                           const std::vector<LocationConfig>& locations)
    : access_log_path_(InitAccessLog(access_log_path)), access_log_level_(access_log_level),
      error_log_path_(InitErrorLogPath(error_log_path)), listeners_(listeners),
      server_names_(InitServerNames(server_names)), locations_(locations)
{}

const std::string& ServerConfig::access_log_path() const
{
    return access_log_path_;
}

Severity ServerConfig::access_log_level() const
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

const std::vector<std::string>& ServerConfig::server_names()
{
    return server_names_;
}

const std::vector<LocationConfig>& ServerConfig::locations()
{
    return locations_;
}

const std::string& ServerConfig::InitAccessLog(const std::string& value)
{
    if (!config::CheckFileExtension(value, ".log") && !value.empty()) {
        throw std::runtime_error("Invalid log file suffix.");
    }
    return value;
}

const std::string& ServerConfig::InitErrorLogPath(const std::string& value)
{
    if (!config::CheckFileExtension(value, ".log") && !value.empty()) {
        throw std::runtime_error("Invalid log file suffix.");
    }
    return value;
}

std::vector<std::pair<in_addr_t, in_port_t> > InitListeners(const std::string& value)
{
    (void)value;
    // TO DO
    return std::vector<std::pair<in_addr_t, in_port_t> >();
}

std::vector<std::string> ServerConfig::InitServerNames(const std::vector<std::string>& value)
{
    (void)value;
    // TO DO
    return std::vector<std::string>();
}

}  // namespace config
