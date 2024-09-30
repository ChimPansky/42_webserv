#include "ServerConfig.h"

#include "c_api/utils.h"
#include "utils.h"

namespace config {

const Severity ServerConfig::kDefaultAccessLogLevel = INFO;
const std::string ServerConfig::kDefaultAccessLogPath = "/log/access.log";
const std::string ServerConfig::kDefaultErrorLogPath = "/log/error.log";
const std::string ServerConfig::kDefaultIPAddress = "localhost";
const in_port_t ServerConfig::kDefaultPort = 80;

ServerConfig::ServerConfig(const std::pair<std::string, Severity>& access_log,
                           const std::string& error_log_path,
                           const std::vector<std::pair<in_addr_t, in_port_t> >& listeners,
                           const std::vector<std::string>& server_names,
                           const std::vector<LocationConfig>& locations)
    : access_log_path_(InitAccessLog(access_log.first)), access_log_level_(access_log.second),
      error_log_path_(InitErrorLogPath(error_log_path)), listeners_(InitListeners(listeners)),
      server_names_(server_names), locations_(locations)
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

const std::vector<std::string>& ServerConfig::server_names() const
{
    return server_names_;
}

const std::vector<LocationConfig>& ServerConfig::locations() const
{
    return locations_;
}

const std::string& ServerConfig::InitAccessLog(const std::string& value)
{
    if (!value.empty() && !config::CheckFileExtension(value, ".log")) {
        throw std::runtime_error("Invalid log file suffix.");
    }
    return value;
}

const std::string& ServerConfig::InitErrorLogPath(const std::string& value)
{
    if (!value.empty() && !config::CheckFileExtension(value, ".log")) {
        throw std::runtime_error("Invalid log file suffix.");
    }
    return value;
}

const std::vector<std::pair<in_addr_t, in_port_t> >& ServerConfig::InitListeners(
    const std::vector<std::pair<in_addr_t, in_port_t> >& value)
{
    for (size_t i = 0; i < value.size(); i++) {
        if (value[i].second == 0) {
            throw std::runtime_error("Invalid configuration file: invalid listener.");
        }
    }
    return value;
}

void ServerConfig::Print() const
{
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "--Server configuration: --";
    LOG(DEBUG) << "Access log path: " << access_log_path_;
    LOG(DEBUG) << "Access log level: " << access_log_level_;
    LOG(DEBUG) << "Error log path: " << error_log_path_;
    LOG(DEBUG) << "Listeners: ";
    for (size_t i = 0; i < listeners_.size(); i++) {
        LOG(DEBUG) << "  " << c_api::IPv4ToString(listeners_[i].first) << ":"
                   << listeners_[i].second;
    }
    LOG(DEBUG) << "Server names:";
    for (size_t i = 0; i < server_names_.size(); i++) {
        LOG(DEBUG) << "  " << server_names_[i];
    }
    for (size_t i = 0; i < locations_.size(); i++) {
        locations_[i].Print();
    }
}
}  // namespace config
