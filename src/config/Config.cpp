#include "Config.h"

#include "ConfigBuilder.h"
#include "ConfigParser.h"
namespace config {

const MxType Config::kDefaultMxType = c_api::EventManager::MT_SELECT;
const std::string Config::kDefaultErrorLogPath = "/log/error.log";
const Severity Config::kDefaultErrorLogLevel = INFO;

Config::Config(MxType mx_type, const std::string& error_log_path, Severity error_log_level,
               const HttpConfig& http_config)
    : mx_type_(mx_type), error_log_path_(InitErrorLogPath(error_log_path)),
      error_log_level_(error_log_level), http_config_(http_config)
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
    if (!(config::CheckFileExtension(config_path, ".conf"))) {
        throw std::invalid_argument("Invalid config file suffix.");
    }

    std::ifstream config_file(config_path.c_str());
    if (!config_file.is_open()) {
        throw std::invalid_argument("Couldn't open config file.");
    }

    ConfigParser parser(config_file, "", "");
    return ConfigBuilder<Config>::Build(parser);
}

}  // namespace config
