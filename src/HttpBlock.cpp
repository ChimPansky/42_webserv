#include "HttpBlock.h"

HttpBlock::HttpBlock()
  : _root(""),
    _index("index.html"),
    _autoindex(false),
    _client_max_body_size(1),
    _keepalive_timeout(65),
    _error_pages()
{}

HttpBlock::HttpBlock(const HttpBlock& copy)
  : _root(copy._root),
    _index(copy._index),
    _autoindex(copy._autoindex),
    _client_max_body_size(copy._client_max_body_size),
    _keepalive_timeout(copy._keepalive_timeout),
    _error_pages(copy._error_pages)
{}

HttpBlock& HttpBlock::operator=(const HttpBlock& copy)
{
    if (this != &copy)
    {
        _root = copy._root;
        _index = copy._index;
        _autoindex = copy._autoindex;
        _client_max_body_size = copy._client_max_body_size;
        _keepalive_timeout = copy._keepalive_timeout;
        _error_pages = copy._error_pages;
    }
    return *this;
}

HttpBlock::~HttpBlock()
{}

const std::string& HttpBlock::root() const
{
    return _root;
}

const std::string& HttpBlock::index() const
{
    return _index;
}

bool HttpBlock::autoindex() const
{
    return _autoindex;
}

int HttpBlock::client_max_body_size() const
{
    return _client_max_body_size;
}

int HttpBlock::keepalive_timeout() const
{
    return _keepalive_timeout;
}

const std::map<int, std::string>& HttpBlock::error_pages()
{
    return _error_pages;
}
