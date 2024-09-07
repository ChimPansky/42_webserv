#ifndef WS_SERVERCONFIG_H
#define WS_SERVERCONFIG_H

#include "LocationConfig.h"
#include "utils/shared_ptr.h"

class   ServerConfig : public IConfig {

  private:
    ServerConfig();
    typedef std::pair<std::string, std::string> Setting;
    void InitAccessLog(const std::string& value);
    void InitErrorLogPath(const std::string& value);
    void InitListeners(const std::string& value);
    void InitRootDir(const std::string& value);
    void InitDefaultFile(const std::string& value);
    void InitDirListing(const std::string& value);
    void InitServerNames(const std::string& value);
    void InitLocations(const std::string& value);

  public:
    ServerConfig(std::vector<Setting> settings);
    ~ServerConfig();
    const std::string& access_log_path() const;
    const std::string& access_log_level() const;
    const std::string& error_log_path() const;
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners() const;
    const std::string& root_dir();
    const std::string& default_file();
    const std::string& dir_listing();
    const std::vector<std::string>& server_names();
    const std::map<std::string, utils::shared_ptr<LocationConfig> >& locations();

  private:
    std::string access_log_path_;
    std::string access_log_level_;
    std::string error_log_path_;
    std::vector<std::pair<in_addr_t, in_port_t> > listeners_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
    std::vector<std::string> server_names_;
    std::map<std::string, utils::shared_ptr<LocationConfig> > locations_;
};

#endif  // WS_SERVERCONFIG_H
