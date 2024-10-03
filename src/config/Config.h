#ifndef WS_CONFIG_CONFIG_H
#define WS_CONFIG_CONFIG_H

#include "HttpConfig.h"
#include "c_api/multiplexers/IMultiplexer.h"

namespace config {

class Config : public IConfigBuilder<Config> {
  private:
    typedef c_api::MultiplexType MxType;

    const std::string& InitErrorLogPath(const std::string& value);

    static bool IsKeyAllowed(const std::string& key);
    static bool IsNestingAllowed(const ParsedConfig& f);

  public:
    Config(MxType mx_type, const std::pair<std::string, Severity>& error_log,
           const HttpConfig& http_config);
    static Config Build(const ParsedConfig& f, const InheritedSettings& inherited_settings);

    MxType mx_type() const;
    const std::string& error_log_path() const;
    Severity error_log_level() const;
    const HttpConfig& http_config() const;

    static const Config GetConfig(const std::string& config_path);
    static inline MxType kDefaultMxType() { return c_api::MT_SELECT; }
    static inline const char* kDefaultErrorLogPath() { return ""; }
    static inline Severity kDefaultErrorLogLevel() { return INFO; }

    void Print() const;

  private:
    MxType mx_type_;
    std::string error_log_path_;
    Severity error_log_level_;
    HttpConfig http_config_;
};

}  // namespace config

#endif  // WS_CONFIG_CONFIG_H
