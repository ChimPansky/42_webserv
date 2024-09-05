#include "ServerBlock.h"

ServerBlock::ServerBlock(std::vector<S>)
    : access_log_path_("/var/log/access.log"),
      access_log_level_("info"),
      error_log_path_(""),
      root_dir_(""),
      default_file_(""),
      dir_listing_("")
{
    server_names_.push_back("localhost");
    // init server settings just like in Config
}

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

const std::string& ServerBlock::dir_listing()
{
    return dir_listing_;
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

std::vector<ServerBlock::S> ServerBlock::ExtractBlock(std::vector<S>::iterator& it)
{
    std::vector<S> block_settings;
    int braces_counter = 1;

    while (braces_counter > 0) {
        it++;
        if (it->second.find("{") != std::string::npos) {
            braces_counter++;
        }
        if (it->first == "}") {
            braces_counter--;
            if (braces_counter == 0) {
                break;
            }
        }
        block_settings.push_back(*it);
    }

    return block_settings;
}
