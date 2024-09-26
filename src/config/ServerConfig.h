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
    const std::vector<std::string>& server_names();
    const std::vector<LocationConfig>& locations();
    static const Severity kDefaultAccessLogLevel;
    static const std::string kDefaultAccessLogPath;
    static const std::string kDefaultErrorLogPath;
    static const std::string kDefaultIPAddress;
    static const in_port_t kDefaultPort;

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
