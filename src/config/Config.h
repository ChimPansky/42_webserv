#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include "HttpConfig.h"
#include "c_api/EventManager.h"

class Config {

  private:
    typedef c_api::EventManager::MultiplexType MxType;
    static const MxType kDefaultMxType;
    static const std::string kDefaultErrorLogPath;
    static const std::string kDefaultErrorLogLevel;
    MxType  InitMxType(const std::string& value);
    const std::string&  InitErrorLogPath(const std::string& value);
    const std::string&  InitErrorLogLevel(const std::string& value);

  public:
    Config(MxType mx_type, const std::string& error_log_path, const std::string& error_log_level, const HttpConfig& http_config);
    MxType mx_type() const;
    const std::string& error_log_path() const;
    const std::string& error_log_level() const;

  private:
    MxType  mx_type_;
    std::string error_log_path_;
    std::string error_log_level_;
    HttpConfig  http_config_;
};

#endif  // WS_CONFIG_H
