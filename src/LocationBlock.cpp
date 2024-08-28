#include "LocationBlock.h"
#include "HttpBlock.h"

LocationBlock::LocationBlock(const HttpBlock& http_block)
  : HttpBlock(http_block),
    _allow_methods(),
    _return(0, ""),
    _cgi(false),
    _cgi_path(""),
    _cgi_extension("")
{
    _allow_methods.push_back("GET");
    _allow_methods.push_back("POST");
}

const std::vector<std::string>& LocationBlock::allow_methods() const
{
    return _allow_methods;
}

const std::pair<int, std::string>& LocationBlock::return_code() const
{
    return _return;
}

bool LocationBlock::cgi() const
{
    return _cgi;
}

const std::string& LocationBlock::cgi_path() const
{
    return _cgi_path;
}

const std::string& LocationBlock::cgi_extension() const
{
    return _cgi_extension;
}
