#ifndef WS_CONFIG_CONFIG_H
#define WS_CONFIG_CONFIG_H

#include "HttpConfig.h"
#include "c_api/EventManager.h"

namespace config {
class Config {
  private:
    typedef c_api::EventManager::MultiplexType MxType;
    const std::string& InitErrorLogPath(const std::string& value);

  public:
    Config(MxType mx_type, const std::string& error_log_path, Severity error_log_level,
           const HttpConfig& http_config);
    MxType mx_type() const;
    const std::string& error_log_path() const;
    Severity error_log_level() const;
    static const Config GetConfig(const std::string& config_path);
    static const MxType kDefaultMxType;
    static const std::string kDefaultErrorLogPath;
    static const Severity kDefaultErrorLogLevel;

    void Print() const;

  private:
    MxType mx_type_;
    std::string error_log_path_;
    Severity error_log_level_;
    HttpConfig http_config_;
};

}  // namespace config

#endif  // WS_CONFIG_CONFIG_H
