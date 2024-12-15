#ifndef WS_SERVER_LOCATION_H
#define WS_SERVER_LOCATION_H

#include <LocationConfig.h>
#include <Request.h>
#include <unique_ptr.h>

class Location {
  public:
    Location(const config::LocationConfig&);
    static std::pair<std::string /*path_in_uri*/, bool /*is_exact_match*/> MatchUriPath(
        const std::string& path, const std::pair<std::string, bool>& route);
    std::pair<std::string /*path_in_uri*/, bool /*is_exact_match*/> MatchedRoute(
        const http::Request& rq) const;
    const std::pair<std::string /*path*/, bool /*is_exact_match*/>& route() const;
    const std::vector<http::Method>& allowed_methods() const;
    const std::pair<int /*status_code*/, std::string /*new_route*/>& redirect() const;
    bool is_cgi() const;
    const std::vector<std::string>& cgi_paths() const;
    const std::vector<std::string>& cgi_extensions() const;
    const std::string& root_dir() const;
    const std::vector<std::string>& default_files() const;
    bool dir_listing() const;
    unsigned int client_max_body_size() const;
    std::string GetDebugString() const;

  private:
    std::pair<std::string /*path*/, bool /*is_exact_match*/> route_;
    std::vector<http::Method> allowed_methods_;
    std::pair<int /*status_code*/, std::string /*new_route*/> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string root_dir_;
    std::vector<std::string> default_files_;
    bool dir_listing_;
    unsigned int client_max_body_size_;
};

#endif  // WS_SERVER_LOCATION_H
