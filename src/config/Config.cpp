#include "Config.h"
#include <iostream>

Config::Config(MxType mx_type, const std::string& error_log_path, Severity error_log_level, const HttpConfig& http_config)
  : mx_type_(mx_type),
    error_log_path_(InitErrorLogPath(error_log_path)),
    error_log_level_(error_log_level),
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

const std::string& Config::InitErrorLogPath(const std::string& value)
{
    (void)value;
    //  TODO
}
