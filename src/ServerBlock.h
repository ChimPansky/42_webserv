#ifndef WS_SERVERBLOCK_H
#define WS_SERVERBLOCK_H

#include <netinet/in.h>
#include <utils/unique_ptr.h>

#include <string>

#include "LocationBlock.h"

typedef std::pair<std::string, std::string> setting;

class ServerBlock {
  public:
    ServerBlock(std::vector<setting> settings);
    const std::string& access_log() const;
    const std::string& access_log_level() const;
    const std::string& error_log_path() const;
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners() const;
    const std::string& root_dir();
    const std::string& default_file();
    bool dir_listing();
    const std::vector<std::string>& server_names();
    // const std::vector<std::pair<std::string, utils::unique_ptr<LocationBlock> > >& locations();
    // utils::unique_ptr<LocationBlock>  FindLocation(const std::string& route);
    static const std::vector<std::string> GetServerTokens();
    void InitListeners() const;
    void InitLocations() const;

  private:
    std::string access_log_;
    std::string access_log_level_;
    std::string error_log_path_;
    std::vector<std::pair<in_addr_t, in_port_t> > listeners_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
    std::vector<std::string> server_names_;
    std::vector<utils::unique_ptr<LocationBlock> > locations_;
};

#endif  // WS_SERVERBLOCK_H
