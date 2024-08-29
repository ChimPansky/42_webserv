#include "ServerBlock.h"

ServerBlock::ServerBlock()
  : access_log_(""),
    access_log_level_("info"),
    port_(8080), 
    root_dir_(""),
    default_file_("index.html"),
    dir_listing_(false),
    client_max_body_size_(1),
    error_pages_(),
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

int ServerBlock::port() const
{
    return port_;
}


const std::string&  ServerBlock::root_dir() const {
    return root_dir_;
}

const std::string&  ServerBlock::default_file() const {
    return default_file_;
}

bool ServerBlock::dir_listing() const {
    return dir_listing_;
}

int ServerBlock::client_max_body_size() const {
    return client_max_body_size_;
}

const std::map<int, std::string>& ServerBlock::error_pages() const {
    return error_pages_;
}

const std::vector<std::string>& ServerBlock::server_names()
{
    return server_names_;
}

const std::list<std::pair<std::string, LocationBlock> >& ServerBlock::locations()
{
    return locations_;
}

const std::vector<std::string>    ServerBlock::GetTokens() {
    
    std::vector<std::string> tokens;

    tokens.push_back("access_log");
    tokens.push_back("listen");
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");
    tokens.push_back("client_max_body_size");
    tokens.push_back("error_page");
    tokens.push_back("server_name");
    tokens.push_back("location");

    return tokens;
}

void    ServerBlock::set_access_log(const std::string& access_log)
{
    access_log_ = access_log;
}

void    ServerBlock::set_access_log_level(const std::string& access_log_level)
{
    access_log_level_ = access_log_level;
}

void    ServerBlock::set_port(int port)
{
    port_ = port;
}

void    ServerBlock::set_root_dir(const std::string& root_dir)
{
    root_dir_ = root_dir;
}

void    ServerBlock::set_default_file(const std::string& default_file)
{
    default_file_ = default_file;
}

void    ServerBlock::set_dir_listing(bool dir_listing)
{
    dir_listing_ = dir_listing;
}

void    ServerBlock::set_client_max_body_size(int client_max_body_size)
{
    client_max_body_size_ = client_max_body_size;
}

void    ServerBlock::set_error_pages(const std::map<int, std::string>& error_pages)
{
    error_pages_ = error_pages;
}
