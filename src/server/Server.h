#ifndef WS_SERVER_SERVER_H
#define WS_SERVER_SERVER_H

#include <Response.h>
#include <ServerConfig.h>
#include <shared_ptr.h>

#include "Location.h"


enum MatchType {
    NO_MATCH = 0,
    SUFFIX_MATCH = 1,
    PREFIX_MATCH = 2,
    EXACT_MATCH = 3
};

enum LocationType {
    CGI,
    STATIC_FILE,
    NO_LOCATION
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
    static std::pair<MatchType, std::string> MatchHostName(const std::string& host,
                                                           const std::vector<std::string>&);
    std::pair<MatchType, std::string> MatchedServerName(const http::Request& rq) const;
    // has to call IResponseCallback with rs when the last is rdy
    utils::unique_ptr<AResponseProcessor> ProcessRequest(
        const http::Request& rq, utils::unique_ptr<http::IResponseCallback> cb) const;
    utils::unique_ptr<AResponseProcessor> GetResponseProcessor(
        const http::Request& rq, utils::unique_ptr<http::IResponseCallback> cb) const;

    std::string name() const;
    const std::vector<std::string>& server_names() const;
    const std::string& access_log_path() const;
    Severity access_log_level() const;
    const std::string& error_log_path() const;
    const std::vector<utils::shared_ptr<Location> >& locations() const;
    std::string GetDebugString() const;

  private:
    std::string access_log_path_;
    Severity access_log_level_;
    std::string error_log_path_;
    std::vector<std::string> server_names_;

    std::vector<utils::shared_ptr<Location> > locations_;
    typedef std::vector<utils::shared_ptr<Location> >::const_iterator LocationsConstIt;

    std::pair<utils::shared_ptr<Location>, LocationType> ChooseLocation(
        const http::Request& rq) const;
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
