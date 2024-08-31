#include "ServerBlock.h"
#include <string>

ServerBlock::ServerBlock()
  : access_log_(""),
    access_log_level_("info"),
    listener_(), 
    root_dir_(""),
    default_file_("index.html"),
    dir_listing_("off"),
    server_names_(),
    locations_()
{}

const std::string&  ServerBlock::access_log() const
{
    return access_log_;
}

const std::string&  ServerBlock::access_log_level() const
{
    return access_log_level_;
}

const std::pair<in_addr_t, in_port_t>&  ServerBlock::listener() const
{
    return listener_;
}


const std::string&  ServerBlock::root_dir(const std::string& route) {

    if (this->FindLocation(route)->root_dir().empty()) {
        return root_dir_;
    }
    return this->FindLocation(route)->root_dir();
}

const std::string&  ServerBlock::default_file(const std::string& route) {

    if (this->FindLocation(route)->default_file().empty()) {
        return default_file_;
    }
    return this->FindLocation(route)->default_file();
}

bool    ServerBlock::dir_listing(const std::string& route) {

    std::string ret = "off";
    if (this->FindLocation(route)->dir_listing().empty()) {
        ret = dir_listing_;
    } else {
        ret = this->FindLocation(route)->dir_listing();
    }
    if (ret == "on") {
        return true;
    }
    return false;
}

const std::vector<std::string>& ServerBlock::server_names()
{
    return server_names_;
}

/* const std::vector<std::pair<std::string, utils::unique_ptr<LocationBlock> > >& ServerBlock::locations()
{
    return locations_; 
} */

utils::unique_ptr<LocationBlock>  ServerBlock::FindLocation(const std::string& route)
{
    std::vector<std::pair<std::string, utils::unique_ptr<LocationBlock> > >::iterator it;

    for (it = locations_.begin(); it != locations_.end(); it++) {
        if (it->first == route) {
            return it->second;
        }
    }
    return locations_.rbegin()->second;
}

const std::vector<std::string>    ServerBlock::GetTokens() {
    
    std::vector<std::string> tokens;

    tokens.push_back("access_log");
    tokens.push_back("listen");
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");
    tokens.push_back("server_name");
    tokens.push_back("location");

    return tokens;
}
