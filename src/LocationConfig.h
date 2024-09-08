#ifndef WS_LOCATIONCONFIG_H
#define WS_LOCATIONCONFIG_H

#include "IConfig.h"

#include <netinet/in.h>
#include <string>
#include <vector>

class   LocationConfig : public IConfig {

  private:
    typedef std::pair<std::string, std::string> Setting;
    void InitRoute(const std::string& value);
    void InitAllowedMethods(const std::string& value);
    void InitRedirect(const std::string& value);
    void InitCgiPaths(const std::string& value);
    void InitCgiExtensions(const std::string& value);
    void InitRootDir(const std::string& value);
    void InitDefaultFile(const std::string& value);
    void InitDirListing(const std::string& value);
    virtual bool  IsValid() const;

  public:
    LocationConfig(const std::vector<Setting>& settings, const std::string& lvl_descrt);
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
