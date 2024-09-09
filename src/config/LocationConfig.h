#ifndef WS_LOCATIONCONFIG_H
#define WS_LOCATIONCONFIG_H

#include <netinet/in.h>
#include <string>
#include <vector>

class   LocationConfig {

  private:
    const std::string&  InitRoute(const std::string& value);
    const std::vector<std::string>& InitAllowedMethods(const std::string& value);
    const std::pair<int, std::string>&  InitRedirect(const std::string& value);
    bool  InitCgi();
    const std::vector<std::string>& InitCgiPaths(const std::string& value);
    const std::vector<std::string>& InitCgiExtensions(const std::string& value);
    const std::string&  InitRootDir(const std::string& value);
    const std::string&  InitDefaultFile(const std::string& value);
    const std::string&  InitDirListing(const std::string& value);

  public:
    LocationConfig(const std::string& route, const std::vector<std::string>& allowed_methods,
                   const std::pair<int, std::string>& redirect, const std::vector<std::string>& cgi_paths, const std::vector<std::string>& cgi_extensions,
                   const std::string& root_dir, const std::string& default_file, const std::string& dir_listing);
    const std::string& route() const;
    const std::vector<std::string>& allowed_methods() const;
    const std::pair<int, std::string>& redirect() const;
    bool is_cgi() const;
    const std::vector<std::string>& cgi_paths() const;
    const std::vector<std::string>& cgi_extensions() const;
    const std::string& root_dir();
    const std::string& default_file() const;
    const std::string& dir_listing() const;

  private:
    std::string route_;
    std::vector<std::string> allowed_methods_;
    std::pair</* status code */ int, /* new route */ std::string> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
};

#endif  // WS_LOCATIONCONFIG_H
