#include "Config.h"

#include "ConfigBuilder.h"
#include "ConfigParser.h"
namespace config {

const MxType Config::kDefaultMxType = c_api::EventManager::MT_SELECT;
const std::string Config::kDefaultErrorLogPath = "/log/error.log";
const Severity Config::kDefaultErrorLogLevel = INFO;

Config::Config(MxType mx_type, const std::pair<std::string, Severity>& error_log,
               const HttpConfig& http_config)
    : mx_type_(mx_type), error_log_path_(InitErrorLogPath(error_log.first)),
      error_log_level_(error_log.second), http_config_(http_config)
{}

Config::MxType Config::mx_type() const
{
    return mx_type_;
}

const std::string& Config::error_log_path() const
{
    return error_log_path_;
}

Severity Config::error_log_level() const
{
    return error_log_level_;
}

const std::string& Config::InitErrorLogPath(const std::string& value)
{
    if (!config::CheckFileExtension(value, ".log")) {
        throw std::runtime_error("Invalid log file suffix.");
    }
    return value;
}

const Config Config::GetConfig(const std::string& config_path)
{
    if (!config::CheckFileExtension(config_path, ".conf")) {
        throw std::invalid_argument("Invalid config file suffix.");
    }

    std::ifstream config_file(config_path.c_str());
    if (!config_file.is_open()) {
        throw std::invalid_argument("Couldn't open config file.");
    }

    ConfigParser parser(config_file, "", "");
    return ConfigBuilder<Config>::Build(parser);
}

void Config::Print() const
{
    LOG(DEBUG) << "--Configuration: --";
    LOG(DEBUG) << "Multiplex type: " << mx_type_;
    LOG(DEBUG) << "Error log path: " << error_log_path_;
    LOG(DEBUG) << "Error log level: " << error_log_level_;
    http_config_.Print();
}

}  // namespace config
