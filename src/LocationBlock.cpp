#include "LocationBlock.h"

LocationBlock::LocationBlock()
  : allow_methods_(),
    return_code_(0, ""),
    cgi_(false),
    cgi_path_(),
    cgi_extension_(),
    root_dir_(""),
    default_file_("index.html"),
    dir_listing_(false),
    client_max_body_size_(1),
    error_pages_()
{
    allow_methods_.push_back("GET");
    allow_methods_.push_back("POST");
}

const std::vector<std::string>& LocationBlock::allow_methods() const
{
    return allow_methods_;
}

const std::pair<int, std::string>& LocationBlock::return_code() const
{
    return return_code_;
}

bool LocationBlock::cgi() const
{
    return cgi_;
}

const std::vector<std::string>& LocationBlock::cgi_path() const
{
    return cgi_path_;
}

const std::vector<std::string>& LocationBlock::cgi_extension() const
{
    return cgi_extension_;
}

const std::string&  LocationBlock::root_dir() const {
    return root_dir_;
}

const std::string&  LocationBlock::default_file() const {
    return default_file_;
}

bool LocationBlock::dir_listing() const {
    return dir_listing_;
}

int LocationBlock::client_max_body_size() const {
    return client_max_body_size_;
}

const std::map<int, std::string>& LocationBlock::error_pages() const {
    return error_pages_;
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
    tokens.push_back("client_max_body_size");
    tokens.push_back("error_page");

    return tokens;
}

void    LocationBlock::set_allow_methods(const std::vector<std::string>& allow_methods)
{
    allow_methods_ = allow_methods;
}

void    LocationBlock::set_return_code(const std::pair<int, std::string>& return_code)
{
    return_code_ = return_code;
}

void    LocationBlock::set_cgi(bool cgi)
{
    cgi_ = cgi;
}

void    LocationBlock::set_cgi_path(const std::vector<std::string>& cgi_path)
{
    cgi_path_ = cgi_path;
}

void    LocationBlock::set_cgi_extension(const std::vector<std::string>& cgi_extension)
{
    cgi_extension_ = cgi_extension;
}

void    LocationBlock::set_root_dir(const std::string& root_dir)
{
    root_dir_ = root_dir;
}

void    LocationBlock::set_default_file(const std::string& default_file)
{
    default_file_ = default_file;
}

void    LocationBlock::set_dir_listing(bool dir_listing)
{
    dir_listing_ = dir_listing;
}

void    LocationBlock::set_client_max_body_size(int client_max_body_size)
{
    client_max_body_size_ = client_max_body_size;
}

void    LocationBlock::set_error_pages(const std::map<int, std::string>& error_pages)
{
    error_pages_ = error_pages;
}
