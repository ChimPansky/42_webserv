#ifndef WS_SERVER_SERVER_H
#define WS_SERVER_SERVER_H

#include <vector>
#include <string>

#include "config/Config.h"
#include "http/Request.h"
#include "http/Response.h"
#include <utils/unique_ptr.h>

// TODO rm
namespace config {
class ServerBlock{};
class LocationBlock{};
}

class Server {
  private:
    Server();
    Server(const Server&);
    Server& operator=(const Server&);

  public:
    // create master socket, register read callback for master socket in event manager
    Server(const config::ServerBlock&);
    Server(const std::string& name);

  public:
    // only check hostname probably.
    bool DoesMatchTheRequest(const http::Request& rq) const;

    // return response with filled headers
    // if has body, rs is in |not ready| state, and body is updated with callbacks
    utils::unique_ptr<http::Response> AcceptRequest(const http::Request& rq);
    // chek the rq, return error possibly
    // find the location, return 404 possibly
    // if location is cgi, run cgi process
    // if it is file, read file

    const std::string& name();

  private:
    std::vector<config::LocationBlock> locations_;
    std::string name_;
};

#endif  // WS_SERVER_SERVER_H
