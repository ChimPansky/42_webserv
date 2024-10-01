#ifndef WS_SERVERCONFIG_H
#define WS_SERVERCONFIG_H

#include <netinet/in.h>

#include "LocationConfig.h"

namespace config {

class ServerConfig {
  private:
    const std::string& InitAccessLog(const std::string& value);
    const std::string& InitErrorLogPath(const std::string& value);
    std::vector<std::string> InitServerNames(const std::vector<std::string>& value);
    const std::vector<std::pair<in_addr_t, in_port_t> >& InitListeners(
        const std::vector<std::pair<in_addr_t, in_port_t> >& value);

  public:
    ServerConfig(const std::pair<std::string, Severity>& error_log,
                 const std::string& error_log_path,
                 const std::vector<std::pair<in_addr_t, in_port_t> >& listeners,
                 const std::vector<std::string>& server_names,
                 const std::vector<LocationConfig>& locations);
    const std::string& access_log_path() const;
    Severity access_log_level() const;
    const std::string& error_log_path() const;
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners() const;
    const std::vector<std::string>& server_names() const;
    const std::vector<LocationConfig>& locations() const;
    static inline Severity kDefaultAccessLogLevel() { return INFO; }
    static inline const char* kDefaultAccessLogPath() { return ""; }
    static inline const char* kDefaultErrorLogPath() { return ""; }
    static inline const char* kDefaultIPAddress() { return "localhost"; }
    static inline in_port_t kDefaultPort() { return 80; }

    void Print() const;

  private:
    std::string access_log_path_;
    Severity access_log_level_;
    std::string error_log_path_;
    std::vector<std::pair<in_addr_t, in_port_t> > listeners_;
    std::vector<std::string> server_names_;
    std::vector<LocationConfig> locations_;
};

}  // namespace config

#endif  // WS_SERVERCONFIG_H
