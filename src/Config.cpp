#include "Config.h"

const std::string Config::kDefaultMxType = "select";
const std::string Config::kDefaultErrorLogPath = "/logs/error.log";
const std::string Config::kDefaultErrorLogLevel = "info";

Config::Config(std::vector<Setting> settings)
{
    for (auto& setting : settings) {
        auto it = InitSettings().find(setting.first);
        if (it != InitSettings().end()) {
            (this->*(it->second))(setting.second);
        } else {
            throw std::runtime_error("Invalid configuration file: unknown setting: " + setting.first);
        }
    }
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

void Config::InitMxType(const std::string& value)
{
    if (value == "select" || value == "poll" || value == "epoll") {
        mx_type_ = value;
    } else if (value.empty()) {
        mx_type_ = kDefaultMxType;
    } else {
        throw std::runtime_error("Invalid configuration file: invalid mx_type: " + value);
    }
}

void Config::InitErrorLog(const std::string& value)
{
    (void)value;
    //  TODO
}

std::map<std::string, Config::FunctionPointer> Config::InitSettings()
{

}
