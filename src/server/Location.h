#ifndef WS_SERVER_LOCATION_H
#define WS_SERVER_LOCATION_H

#include <IResponseProcessor.h>
#include <LocationConfig.h>
#include <Request.h>
#include <unique_ptr.h>

class Location {
  private:
    std::pair<std::string, bool> route_;
    std::vector<http::Method> allowed_methods_;
    std::pair<int /* status code */, std::string /* new route */> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string root_dir_;
    std::vector<std::string> default_file_;
    bool dir_listing_;
    unsigned int client_max_body_size_;

  public:
    Location();
    Location(const config::LocationConfig&);
    static std::pair<bool /* is exact match */, std::string> MatchUriPath(
        const std::string& path, const std::pair<std::string, bool>& route);
    std::pair<bool, std::string> MatchedRoute(const http::Request& rq) const;
    const std::pair<std::string, /* is exact match */ bool>& route() const;
    const std::vector<http::Method>& allowed_methods() const;
    const std::pair<int, std::string>& redirect() const;
    const std::string& root_dir() const;
    const std::vector<std::string>& default_file() const;
    unsigned int client_max_body_size() const;
    std::string GetInfo() const;
    utils::unique_ptr<AResponseProcessor> GetResponseProcessor(
        utils::unique_ptr<http::IResponseCallback> response_rdy_cb, const http::Request& rq) const;
};

#endif  // WS_SERVER_LOCATION_H
