#ifndef WS_SERVER_LOCATION_H
#define WS_SERVER_LOCATION_H

#include <IResponseProcessor.h>
#include <LocationConfig.h>
#include <Request.h>
#include <unique_ptr.h>

class Location {
  private:
    std::pair<std::string /* path */, bool /* is exact match */> route_;
    std::vector<http::Method> allowed_methods_;
    std::pair<int /* status code */, std::string /* new route */> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string root_dir_;
    std::vector<std::string> default_files_;
    bool dir_listing_;
    unsigned int client_max_body_size_;

  public:
    Location(const config::LocationConfig&);
    static std::pair<std::string /* path in uri */, bool /* is exact match */> MatchUriPath(
        const std::string& path, const std::pair<std::string, bool>& route);
    std::pair<std::string /* path in uri */, bool /* is exact match */> MatchedRoute(
        const http::Request& rq) const;
    const std::pair<std::string /* path */, bool /* is exact match */>& route() const;
    const std::vector<http::Method>& allowed_methods() const;
    const std::pair<int /* status code */, std::string /* new route */>& redirect() const;
    bool is_cgi() const;
    const std::vector<std::string>& cgi_paths() const;
    const std::vector<std::string>& cgi_extensions() const;
    const std::string& root_dir() const;
    const std::vector<std::string>& default_files() const;
    bool dir_listing() const;
    unsigned int client_max_body_size() const;
};

std::ostream& operator<<(std::ostream& oss, const Location& loc);

#endif  // WS_SERVER_LOCATION_H
