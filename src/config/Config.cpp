#include "Config.h"

#include "ConfigBuilder.h"
#include "ParsedConfig.h"

namespace config {

Config::Config(MxType mx_type, const std::pair<std::string, Severity>& error_log,
               const HttpConfig& http_config)
    : mx_type_(mx_type), error_log_path_(error_log.first), error_log_level_(error_log.second),
      http_config_(http_config)
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

const HttpConfig& Config::http_config() const
{
    return http_config_;
}

const Config Config::GetConfig(const std::string& config_path)
{
    if (!utils::fs::CheckFileExtension(config_path, ".conf") ||
        config_path[config_path.size() - 6] == '/') {
        throw std::invalid_argument("Invalid config file suffix.");
    }

    std::ifstream config_file(config_path.c_str());
    if (!config_file.is_open()) {
        throw std::invalid_argument("Couldn't open config file.");
    }

    ParsedConfig parser(config_file, "", "");
    InheritedSettings inherited_settings = {};
    return ConfigBuilder<Config>::Build(parser, inherited_settings);
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