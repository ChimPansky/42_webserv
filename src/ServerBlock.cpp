#include "ServerBlock.h"

ServerBlock::ServerBlock(const HttpBlock& http_block)
  : HttpBlock(http_block),
    _error_log(),
    _error_log_level(),
    _access_log(),
    _access_log_level(),
    _listen(80),
    _server_names(),
    _locations()
{}

const std::string&  ServerBlock::error_log() const
{
    return _error_log;
}

const std::string&  ServerBlock::error_log_level() const
{
    return _error_log_level;
}

const std::string&  ServerBlock::access_log() const
{
    return _access_log;
}

const std::string&  ServerBlock::access_log_level() const
{
    return _access_log_level;
}

int ServerBlock::listen() const
{
    return _listen;
}

const std::vector<std::string>& ServerBlock::server_names()
{
    return _server_names;
}

const std::list<std::pair<std::string, LocationBlock> >& ServerBlock::locations()
{
    return _locations;
}
