#ifndef WS_SERVERCONFIG_H
#define WS_SERVERCONFIG_H

#include "LocationConfig.h"

class   ServerConfig {

  private:
    const std::string& InitAccessLog(const std::string& value);
    const std::string& InitErrorLogPath(const std::string& value);
    const std::vector<std::pair<in_addr_t, in_port_t> >& InitListeners(const std::string& value);
    const std::string& InitRootDir(const std::string& value);
    const std::string& InitDefaultFile(const std::string& value);
    const std::string& InitDirListing(const std::string& value);
    const std::vector<std::string>& InitServerNames(const std::string& value);
    const std::map<std::string, LocationConfig>& InitLocations(const std::string& value);

  public:
    ServerConfig(const std::string& access_log_path, const std::string& access_log_level, const std::string& error_log_path,
                 const std::vector<std::pair<in_addr_t, in_port_t> >& listeners, const std::string& root_dir,
                 const std::string& default_file, const std::string& dir_listing, const std::vector<std::string>& server_names,
                 const std::map<std::string, LocationConfig>& locations);
    const std::string& access_log_path() const;
    const std::string& access_log_level() const;
    const std::string& error_log_path() const;
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners() const;
    const std::string& root_dir();
    const std::string& default_file();
    const std::string& dir_listing();
    const std::vector<std::string>& server_names();
    const std::map<std::string, LocationConfig>& locations();

  private:
    std::string access_log_path_;
    std::string access_log_level_;
    std::string error_log_path_;
    std::vector<std::pair<in_addr_t, in_port_t> > listeners_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
    std::vector<std::string> server_names_;
    std::map<std::string, LocationConfig> locations_;
};

#endif  // WS_SERVERCONFIG_H
