#ifndef WS_SERVER_LOCATION_H
#define WS_SERVER_LOCATION_H

#include <LocationConfig.h>
#include <Request.h>

class Location {
  private:
    std::pair<std::string, bool> route_;
    std::vector<config::LocationConfig::Method> allowed_methods_;
    std::pair<int /* status code */, std::string /* new route */> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string root_dir_;
    std::vector<std::string> default_file_;
    unsigned int client_max_body_size_;

  public:
    Location();
    Location(const config::LocationConfig&);
    std::pair<bool /* is exact match */, std::string> MatchedRoute(const http::Request& rq) const;
    const std::pair<std::string, /* is exact match */ bool>& route() const;
    const std::vector<config::LocationConfig::Method>& allowed_methods() const;
    const std::pair<int, std::string>& redirect() const;
    bool is_cgi() const;
    const std::vector<std::string>& cgi_paths() const;
    const std::vector<std::string>& cgi_extensions() const;
    const std::string& root_dir() const;
    const std::vector<std::string>& default_file() const;
    unsigned int client_max_body_size() const;
};

#endif  // WS_SERVER_LOCATION_H
