#include "LocationBlock.h"

LocationBlock::LocationBlock(const std::vector<setting>& settings)
    : allowed_methods_(),
      redirect_(0, ""),
      is_cgi_(false),
      cgi_paths_(),
      cgi_extensions_(),
      root_dir_(""),
      default_file_("index.html"),
      dir_listing_("")
{
    allowed_methods_.push_back("GET");
    allowed_methods_.push_back("POST");
    // parse values from the map
    (void)settings;
}

const std::string& LocationBlock::route() const
{
    return route_;
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

const std::vector<std::string>& LocationBlock::cgi_paths() const
{
    return cgi_paths_;
}

const std::vector<std::string>& LocationBlock::cgi_extensions() const
{
    return cgi_extensions_;
}

const std::string& LocationBlock::root_dir()
{
    return root_dir_;
}

const std::string& LocationBlock::default_file() const
{
    return default_file_;
}

const std::string& LocationBlock::dir_listing() const
{
    return dir_listing_;
}

const std::vector<std::string> LocationBlock::GetTokens()
{
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
