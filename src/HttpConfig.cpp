#include "HttpConfig.h"

const int HttpConfig::kDefaultKeepaliveTimeout = 65;

size_t HttpConfig::kDefaultClientMaxBodySize = 1048576; // 1MB

const std::string HttpConfig::kDefaultDefaultFile = "index.html";

const std::string HttpConfig::kDefaultDirListing = "off";


HttpConfig::HttpConfig(std::vector<Setting> settings)
{


}

int HttpConfig::keepalive_timeout() const
{
    return keepalive_timeout_;
}

size_t HttpConfig::client_max_body_size() const
{
    return client_max_body_size_;
}

const std::map<int, std::string>& HttpConfig::error_pages() const
{
    return error_pages_;
}

const std::string& HttpConfig::root_dir() const
{
    return root_dir_;
}

const std::string& HttpConfig::default_file() const
{
    return default_file_;
}

const std::string& HttpConfig::dir_listing() const
{
    return dir_listing_;
}

const std::vector<ServerConfig>& HttpConfig::server_configs() const
{
    return server_configs_;
}

void HttpConfig::InitServers(const std::vector<Setting>& server_settings)
{
    (void)server_settings;
    //  TODO
}

void HttpConfig::InitKeepaliveTimeout(const std::string& value)
{
    (void)value;
    //  TODO
}

void HttpConfig::InitClientMaxBodySize(const std::string& value)
{
    (void)value;
    //  TODO
}

void HttpConfig::InitErrorPages(const std::string& value)
{
    (void)value;
    //  TODO
}

void HttpConfig::InitRootDir(const std::string& value)
{
    (void)value;
    //  TODO
}

void HttpConfig::InitDefaultFile(const std::string& value)
{
    (void)value;
    //  TODO
}

void HttpConfig::InitDirListing(const std::string& value)
{
    (void)value;
    //  TODO
}

/* std::map<std::string, HttpConfig::FunctionPointer> HttpConfig::InitSettings() const
{

} */
