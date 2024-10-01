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
    std::pair<int, std::string> InitRedirect(const std::pair<int, std::string>& value);

  public:
    enum Priority {
        P0 = 0,
        P1 = 1
    };
    LocationConfig(const std::pair<std::string, Priority>& route,
                   const std::vector<std::string>& allowed_methods,
                   const std::pair<int, std::string>& redirect,
                   const std::vector<std::string>& cgi_paths,
                   const std::vector<std::string>& cgi_extensions, const std::string& root_dir,
                   const std::string& default_file, bool dir_listing);
    const std::pair<std::string, Priority>& route() const;
    const std::vector<std::string>& allowed_methods() const;
    const std::pair<int, std::string>& redirect() const;
    bool is_cgi() const;
    const std::vector<std::string>& cgi_paths() const;
    const std::vector<std::string>& cgi_extensions() const;
    const std::string& root_dir() const;
    const std::string& default_file() const;
    bool dir_listing() const;
    static inline int kDefaultRedirectCode()
    {
        return 301;
    }
    static inline const char* kDefaultRedirectPath()
    {
        return "/new_location";
    }
    static inline const char* kDefaultRootDir()
    {
        return "/docs";
    }
    static inline const char* kDefaultIndexFile()
    {
        return "index.html";
    }
    static inline bool kDefaultDirListing()
    {
        return false;
    }

    void Print() const;

  private:
    std::pair<std::string, Priority> route_;
    std::vector<std::string> allowed_methods_;
    std::pair</* status code */ int, /* new route */ std::string> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string root_dir_;
    std::string default_file_;
    bool dir_listing_;
};

}  // namespace config

#endif  // WS_LOCATIONCONFIG_H
