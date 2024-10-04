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
    enum Method {
        GET,
        POST,
        DELETE
    };
    LocationConfig(const std::pair<std::string, /* is exact match */ bool>& route,
                   const std::vector<Method>& allowed_methods,
                   const std::pair<int, std::string>& redirect,
                   const std::vector<std::string>& cgi_paths,
                   const std::vector<std::string>& cgi_extensions, const std::string& root_dir,
                   const std::vector<std::string>& default_file, bool dir_listing);

    const std::pair<std::string, /* is exact match */ bool>& route() const;
    const std::vector<Method>& allowed_methods() const;
    const std::pair<int, std::string>& redirect() const;
    bool is_cgi() const;
    const std::vector<std::string>& cgi_paths() const;
    const std::vector<std::string>& cgi_extensions() const;
    const std::string& root_dir() const;
    const std::vector<std::string>& default_file() const;
    bool dir_listing() const;

    static inline int kDefaultRedirectCode() { return 301; }
    static inline const char* kDefaultRedirectPath() { return "/new_location"; }
    static inline const char* kDefaultRootDir() { return "/docs"; }
    static inline std::vector<std::string> kDefaultIndexFile()
    {
        std::vector<std::string> default_file;
        default_file.push_back("index.html");
        return default_file;
    }
    static inline bool kDefaultDirListing() { return false; }
    static inline std::vector<Method> kDefaultAllowedMethods()
    {
        std::vector<Method> default_methods;
        default_methods.push_back(GET);
        default_methods.push_back(POST);
        return default_methods;
    }
    static inline std::vector<std::string> kDefaultCgiPath()
    {
        std::vector<std::string> default_cgi_paths;
        default_cgi_paths.push_back("/cgi-bin/");
        return default_cgi_paths;
    }
    static inline std::vector<std::string> kDefaultCgiExtensions()
    {
        std::vector<std::string> default_cgi_extensions;
        default_cgi_extensions.push_back(".py");
        default_cgi_extensions.push_back(".php");
        return default_cgi_extensions;
    }

    void Print() const;

  private:
    std::pair<std::string, bool> route_;
    std::vector<Method> allowed_methods_;
    std::pair<int /* status code */, std::string /* new route */> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string root_dir_;
    std::vector<std::string> default_file_;
    bool dir_listing_;
};

}  // namespace config

#endif  // WS_LOCATIONCONFIG_H
