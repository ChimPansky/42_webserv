#include "ServerBlock.h"

ServerBlock::ServerBlock(std::vector<setting>)
    : access_log_path_(""),
      access_log_level_("info"),
      listeners_(),
      root_dir_(""),
      default_file_("index.html"),
      dir_listing_("off"),
      server_names_(),
      locations_()
{}

const std::string& ServerBlock::access_log_path() const
{
    return access_log_path_;
}

const std::string& ServerBlock::access_log_level() const
{
    return access_log_level_;
}

const std::string& ServerBlock::error_log_path() const
{
    return error_log_path_;
}

const std::vector<std::pair<in_addr_t, in_port_t> >& ServerBlock::listeners() const
{
    return listeners_;
}

const std::string& ServerBlock::root_dir()
{
    return root_dir_;
}

const std::string& ServerBlock::default_file()
{
    return this->default_file_;
}

bool ServerBlock::dir_listing()
{
    if (dir_listing_ == "on") {
        return true;
    }
    return false;
}

const std::vector<std::string>& ServerBlock::server_names()
{
    return server_names_;
}

const std::map<std::string, utils::unique_ptr<LocationBlock> >& ServerBlock::locations()
{
    return locations_;
}

/* utils::unique_ptr<LocationBlock>  ServerBlock::FindLocation(const std::string& route)
{
    std::vector<std::pair<std::string, utils::unique_ptr<LocationBlock> > >::iterator it;

    for (it = locations_.begin(); it != locations_.end(); it++) {
        if (it->first == route) {
            return it->second;
        }
    }
    return locations_.rbegin()->second;
} */

const std::vector<std::string> ServerBlock::GetTokens()
{
    std::vector<std::string> tokens;

    tokens.push_back("access_log");
    tokens.push_back("error_log");
    tokens.push_back("listen");
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");
    tokens.push_back("server_name");
    tokens.push_back("location");

    return tokens;
}
