#include "Config.h"
#include <iostream>

const Config::MxType Config::kDefaultMxType = MxType::MT_SELECT;
const std::string Config::kDefaultErrorLogPath = "/logs/error.log";
const std::string Config::kDefaultErrorLogLevel = "info";

Config::Config(MxType mx_type, const std::string& error_log_path, const std::string& error_log_level, const HttpConfig& http_config)
  : mx_type_(mx_type),
    error_log_path_(InitErrorLogPath(error_log_path)),
    error_log_level_(InitErrorLogLevel(error_log_level)),
    http_config_(http_config)
{
}

Config::MxType Config::mx_type() const
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

Config::MxType Config::InitMxType(const std::string& value) // in ConfigBuilder
{
    if (value == "select") {
        return MxType::MT_SELECT;
    } else if (value == "poll") {
        return MxType::MT_POLL;
    } else if (value == "epoll") {
        return MxType::MT_EPOLL;
    } else if (value.empty()) {
        mx_type_ = kDefaultMxType;
    } else {
        throw std::runtime_error("Invalid configuration file: invalid mx_type: " + value);
    }
}

const std::string& Config::InitErrorLogPath(const std::string& value)
{
    (void)value;
    //  TODO
}

const std::string& Config::InitErrorLogLevel(const std::string& value)
{
    (void)value;
    //  TODO
}
