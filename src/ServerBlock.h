#ifndef WS_SERVERBLOCK_H
#define WS_SERVERBLOCK_H

#include <netinet/in.h>
#include <utils/unique_ptr.h>

#include <map>
#include <string>

#include "LocationBlock.h"

typedef std::pair<std::string, std::string> setting;

class ServerBlock {
  public:
    ServerBlock(std::vector<setting> settings);
    const std::string& access_log_path() const;
    const std::string& access_log_level() const;
    const std::string& error_log_path() const;
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners() const;
    const std::string& root_dir();
    const std::string& default_file();
    bool dir_listing();
    const std::vector<std::string>& server_names();
    const std::map<std::string, utils::unique_ptr<LocationBlock> >& locations();
    static const std::vector<std::string> GetTokens();
    void InitListeners() const;
    void InitLocations() const;

  private:
    std::string access_log_path_;
    std::string access_log_level_;
    std::string error_log_path_;
    std::vector<std::pair<in_addr_t, in_port_t> > listeners_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
    std::vector<std::string> server_names_;
    std::map<std::string, utils::unique_ptr<LocationBlock> > locations_;
};

#endif  // WS_SERVERBLOCK_H
