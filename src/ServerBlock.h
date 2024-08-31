#ifndef WS_SERVERBLOCK_H
#define WS_SERVERBLOCK_H

#include <netinet/in.h>
#include "LocationBlock.h"

#include <string>
#include <utils/unique_ptr.h>

class   ServerBlock {

  private:
    std::string access_log_;
    std::string access_log_level_;
    std::pair<in_addr_t, in_port_t> listener_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
    std::vector<std::string> server_names_;
    std::vector<std::pair</* route */std::string, utils::unique_ptr<LocationBlock> > > locations_;

  public:
    ServerBlock(/* std::map<std::string, std::string>, std::map<std::string, std::string>::iterator */);
    const std::string&  access_log() const;
    const std::string&  access_log_level() const;
    const std::pair<in_addr_t, in_port_t>&  listener() const;
    const std::string&  root_dir(const std::string& route);
    const std::string&  default_file(const std::string& route);
    bool  dir_listing(const std::string& route);
    const std::map<int, std::string>& error_pages() const;
    const std::vector<std::string>& server_names();
    //const std::vector<std::pair<std::string, utils::unique_ptr<LocationBlock> > >& locations();
    utils::unique_ptr<LocationBlock>  FindLocation(const std::string& route);
    static const std::vector<std::string>    GetTokens();
};

#endif  // WS_SERVERBLOCK_H
