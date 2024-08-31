#ifndef WS_LOCATIONBLOCK_H
#define WS_LOCATIONBLOCK_H

#include <map>
#include <string>
#include <vector>

class   LocationBlock {

  private:
    std::vector<std::string> allowed_methods_;
    std::pair</* status code */int, /* new route */std::string> return_code_;
    bool is_cgi_;
    std::vector<std::string> cgi_path_;
    std::vector<std::string> cgi_extension_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;

  public:
    LocationBlock(/* std::map<std::string, std::string>, std::map<std::string, std::string>::iterator */);
    const std::vector<std::string>& allowed_methods() const;
    const std::pair<int, std::string>& return_code() const;
    bool  is_cgi() const;
    const std::vector<std::string>& cgi_path() const;
    const std::vector<std::string>& cgi_extension() const;
    const std::string&  root_dir();
    const std::string&  default_file() const;
    const std::string&  dir_listing() const;
    static const std::vector<std::string>    GetTokens();
};

#endif  // WS_LOCATIONBLOCK_H
