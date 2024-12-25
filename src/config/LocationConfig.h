#ifndef WS_LOCATIONCONFIG_H
#define WS_LOCATIONCONFIG_H

#include <ResponseCodes.h>
#include <http.h>
#include <logger.h>

#include <string>
#include <utility>
#include <vector>

namespace config {

class LocationConfig {
  private:
    std::pair<int /*status_code*/, std::string /*new_route*/> InitRedirect(
        const std::pair<int, std::string>& value);

  public:
    LocationConfig(const std::pair<std::string /*path*/, bool /*is_exact_match*/>& route,
                   const std::vector<http::Method>& allowed_methods,
                   const std::pair<int /*status_code*/, std::string /*new_route*/>& redirect,
                   const std::vector<std::string>& cgi_paths,
                   const std::vector<std::string>& cgi_extensions, const std::string& alias_dir,
                   const std::vector<std::string>& default_files, bool dir_listing,
                   unsigned int client_max_body_size, const std::string& upload_dir);

    const std::pair<std::string /*path*/, /*is_exact_match*/ bool>& route() const;
    const std::vector<http::Method>& allowed_methods() const;
    const std::pair<int /*status_code*/, std::string /*new_route*/>& redirect() const;
    bool is_cgi() const;
    const std::vector<std::string>& cgi_paths() const;
    const std::vector<std::string>& cgi_extensions() const;
    const std::string& alias_dir() const;
    const std::vector<std::string>& default_files() const;
    bool dir_listing() const;
    unsigned int client_max_body_size() const;
    const std::string& upload_dir() const;
    static inline const char* kDefaultAliasDir() { return "/docs"; }
    static inline unsigned int kDefaultClientMaxBodySize() { return 2ul << 20; }
    static inline std::vector<std::string> kDefaultIndexFile()
    {
        std::vector<std::string> default_files;
        default_files.push_back("index.html");
        return default_files;
    }
    static inline bool kDefaultDirListing() { return false; }
    static inline std::vector<http::Method> kDefaultAllowedMethods()
    {
        std::vector<http::Method> default_methods;
        default_methods.push_back(http::HTTP_GET);
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
    static inline std::pair<std::string /*path*/, bool /*is_exact_match*/> kDefaultRoute()
    {
        return std::make_pair("/", false);
    }

    void Print() const;

  private:
    std::pair<std::string /*path*/, bool /*is_exact_match*/> route_;
    std::vector<http::Method> allowed_methods_;
    std::pair<int /*status_code*/, std::string /*new_route*/> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string alias_dir_;
    std::vector<std::string> default_files_;
    bool dir_listing_;
    unsigned int client_max_body_size_;
    std::string upload_dir_;
};

}  // namespace config

#endif  // WS_LOCATIONCONFIG_H
