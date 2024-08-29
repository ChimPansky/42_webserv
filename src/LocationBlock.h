#ifndef WS_LOCATIONBLOCK_H
#define WS_LOCATIONBLOCK_H

#include <map>
#include <string>
#include <vector>

class   LocationBlock {

  private:
    std::vector<std::string> allow_methods_;
    std::pair<int, std::string> return_code_;
    bool cgi_;
    std::vector<std::string> cgi_path_;
    std::vector<std::string> cgi_extension_;
    std::string root_dir_;
    std::string default_file_;
    bool dir_listing_;
    int client_max_body_size_;
    std::map<int, std::string> error_pages_;

  public:
    LocationBlock();
    const std::vector<std::string>& allow_methods() const;
    const std::pair<int, std::string>& return_code() const;
    bool cgi() const;
    const std::vector<std::string>& cgi_path() const;
    const std::vector<std::string>& cgi_extension() const;
    const std::string&  root_dir() const;
    const std::string&  default_file() const;
    bool dir_listing() const;
    int client_max_body_size() const;
    const std::map<int, std::string>& error_pages() const;
    static const std::vector<std::string>    GetTokens();
    void  set_allow_methods(const std::vector<std::string>& allow_methods);
    void  set_return_code(const std::pair<int, std::string>& return_code);
    void  set_cgi(bool cgi);
    void  set_cgi_path(const std::vector<std::string>& cgi_path);
    void  set_cgi_extension(const std::vector<std::string>& cgi_extension);
    void  set_root_dir(const std::string& root_dir);
    void  set_default_file(const std::string& default_file);
    void  set_dir_listing(bool dir_listing);
    void  set_client_max_body_size(int client_max_body_size);
    void  set_error_pages(const std::map<int, std::string>& error_pages);
};

#endif  // WS_LOCATIONBLOCK_H
