#ifndef WS_SERVER_SERVER_H
#define WS_SERVER_SERVER_H

#include <LocationConfig.h>
#include <Request.h>
#include <Response.h>
#include <ServerConfig.h>
#include <unique_ptr.h>

#include <string>
#include <vector>

enum MatchType {
    NO_MATCH = 0,
    SUFFIX_MATCH = 1,
    PREFIX_MATCH = 2,
    EXACT_MATCH = 3
};

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
    std::pair<MatchType, std::string> MatchedServerName(const http::Request& rq) const;

    // has to call IResponseCallback with rs when the last is rdy
    void AcceptRequest(const http::Request& rq,
                       utils::unique_ptr<http::IResponseCallback> cb) const;

    std::string name() const;

    const config::ServerConfig& server_config() const { return server_config_; }

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
