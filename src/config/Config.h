#ifndef WS_CONFIG_CONFIG_H
#define WS_CONFIG_CONFIG_H

#include <multiplexers/IMultiplexer.h>

#include "HttpConfig.h"
#include "ServerConfig.h"

namespace config {

typedef std::vector<ServerConfig>::iterator ServerConfIt;
typedef std::vector<ServerConfig>::const_iterator ServerConfConstIt;
typedef std::vector<std::pair<in_addr_t, in_port_t> >::iterator ListenersConfIt;
typedef std::vector<std::pair<in_addr_t, in_port_t> >::const_iterator ListenersConfConstIt;

class Config {
  private:
    typedef c_api::MultiplexType MxType;

    const std::string& InitErrorLogPath(const std::string& value);

  public:
    Config(MxType mx_type, const std::pair<std::string, Severity>& error_log,
           const HttpConfig& http_config);

    MxType mx_type() const;
    const std::string& error_log_path() const;
    Severity error_log_level() const;
    const HttpConfig& http_config() const;

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
