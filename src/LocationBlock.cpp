#include "LocationBlock.h"

LocationBlock::LocationBlock()
  : allowed_methods_(),
    redirect_(0, ""),
    is_cgi_(false),
    cgi_path_(),
    cgi_extension_(),
    root_dir_(""),
    default_file_("index.html"),
    dir_listing_("")
{
    allowed_methods_.push_back("GET");
    allowed_methods_.push_back("POST");
}

const std::vector<std::string>& LocationBlock::allowed_methods() const
{
    return allowed_methods_;
}

const std::pair<int, std::string>& LocationBlock::redirect() const
{
    return redirect_;
}

bool LocationBlock::is_cgi() const
{
    return is_cgi_;
}

const std::vector<std::string>& LocationBlock::cgi_path() const
{
    return cgi_path_;
}

const std::vector<std::string>& LocationBlock::cgi_extension() const
{
    return cgi_extension_;
}

const std::string&  LocationBlock::root_dir() {
    return root_dir_;
}

const std::string&  LocationBlock::default_file() const {
    return default_file_;
}

const std::string&  LocationBlock::dir_listing() const {
    return dir_listing_;
}

const std::vector<std::string>    LocationBlock::GetTokens() {
    
    std::vector<std::string> tokens;

    tokens.push_back("allow_methods");
    tokens.push_back("return");
    tokens.push_back("cgi_path");
    tokens.push_back("cgi_extension");
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");

    return tokens;
}
