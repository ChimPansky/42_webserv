#ifndef WS_SERVERCONFIG_H
#define WS_SERVERCONFIG_H

#include "LocationConfig.h"

#include <map>

namespace config {

class   ServerConfig {

  private:
    const std::string& InitAccessLog(const std::string& value);
    const std::string& InitErrorLogPath(const std::string& value);
    std::vector<std::pair<in_addr_t, in_port_t> > InitListeners(const std::string& value);
    const std::string& InitRootDir(const std::string& value);
    const std::string& InitDefaultFile(const std::string& value);
    std::vector<std::string> InitServerNames(const std::vector<std::string>& value);

  public:
    ServerConfig(const std::string& access_log_path, Severity access_log_level, const std::string& error_log_path,
                 const std::vector<std::pair<in_addr_t, in_port_t> >& listeners, const std::string& root_dir,
                 const std::string& default_file, const std::string& dir_listing, const std::vector<std::string>& server_names,
                 const std::vector<LocationConfig>& locations);
    const std::string& access_log_path() const;
    Severity  access_log_level() const;
    const std::string& error_log_path() const;
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners() const;
    const std::string& root_dir();
    const std::string& default_file();
    const std::string& dir_listing();
    const std::vector<std::string>& server_names();
    const std::vector<LocationConfig>& locations();

  private:
    std::string access_log_path_;
    Severity  access_log_level_;
    std::string error_log_path_;
    std::vector<std::pair<in_addr_t, in_port_t> > listeners_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
    std::vector<std::string> server_names_;
    std::vector<LocationConfig> locations_;
};

}  // namespace config

#endif  // WS_SERVERCONFIG_H
