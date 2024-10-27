#ifndef WS_SERVER_SERVER_H
#define WS_SERVER_SERVER_H

#include <vector>
#include <string>

#include <ServerConfig.h>
#include <LocationConfig.h>
#include <Request.h>
#include <Response.h>
#include <unique_ptr.h>


class Server {
  private:
    Server();
    Server(const Server&);
    Server& operator=(const Server&);

  public:
    // create master socket, register read callback for master socket in event manager
    Server(const config::ServerConfig&);

  public:
    // only check hostname probably.
    bool DoesMatchTheRequest(const http::Request& rq) const;

    // return response with filled headers
    // if has body, rs is in |not ready| state, and body is updated with callbacks
    utils::unique_ptr<http::Response::ResponseBuilder> AcceptRequest(const http::Request& rq) const;
    // chek the rq, return error possibly
    // find the location, return 404 possibly
    // if location is cgi, run cgi process
    // if it is file, read file

    const std::string& name() const;

    const config::ServerConfig& server_config() const {return server_config_;}

  private:
    std::vector<config::LocationConfig> locations_;
    config::ServerConfig server_config_;
};

#endif  // WS_SERVER_SERVER_H


/*

register ClientRecvCallback with client session

ClientRecvCallback when rq is received

choose server for client

server Accept request -> ResponseBodyGenerator or response

if !response:

    register ResponseBodyGenerator callback with client session

    ResponseBodyGenerator callback when body is generated:

    response finalize

Client send response

register ClientSendCallback with response

*/
