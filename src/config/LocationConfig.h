#ifndef WS_LOCATIONCONFIG_H
#define WS_LOCATIONCONFIG_H

#include <netinet/in.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "utils/logger.h"

namespace config {

class LocationConfig {
  private:
    const std::string& InitDefaultFile(const std::string& value);
    std::pair<int, std::string> InitRedirect(const std::pair<int, std::string>& value);

  public:
    enum LocationPriority {
        P0 = 0,
        P1 = 1,
        P2 = 2
    };
    LocationConfig(const std::pair<std::string, std::string>& route, const std::vector<std::string>& allowed_methods,
                   const std::pair<int, std::string>& redirect,
                   const std::vector<std::string>& cgi_paths,
                   const std::vector<std::string>& cgi_extensions, const std::string& root_dir,
                   const std::string& default_file, const std::string& dir_listing);
    const std::pair<std::string, LocationPriority>& route() const;
    const std::vector<std::string>& allowed_methods() const;
    const std::pair<int, std::string>& redirect() const;
    bool is_cgi() const;
    const std::vector<std::string>& cgi_paths() const;
    const std::vector<std::string>& cgi_extensions() const;
    const std::string& root_dir();
    const std::string& default_file() const;
    const std::string& dir_listing() const;
    static const int kDefaultRedirectCode;
    static const std::string kDefaultRedirectPath;
    static const std::string kDefaultRootDir;
    static const std::string kDefaultIndexFile;
    static const std::string kDefaultDirListing;

    void Print() const;

  private:
    const std::pair<std::string, LocationConfig::LocationPriority>  InitRoute(const std::pair<std::string, std::string>& value);
    std::pair<std::string, LocationPriority> route_;
    std::vector<std::string> allowed_methods_;
    std::pair</* status code */ int, /* new route */ std::string> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
};

}  // namespace config

#endif  // WS_LOCATIONCONFIG_H
