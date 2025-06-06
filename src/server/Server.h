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
    STATIC_PATH,
    NO_LOCATION,
    REDIRECT
};

class Server;

struct RequestDestination {
    RequestDestination(utils::shared_ptr<Server> server)
        : server(server), loc(NULL), updated_path("")
    {}
    utils::shared_ptr<Server> server;
    utils::shared_ptr<Location> loc;
    std::string updated_path;
};

class AResponseProcessor;

class Server {
  private:
    Server();
    Server(const Server&);
    Server& operator=(const Server&);

  public:
    // create master socket, register read callback for master socket in event manager
    Server(const config::ServerConfig&, std::map<int, std::string> error_pages);

  public:
    // only check hostname probably.
    static std::pair<MatchType, std::string> MatchHostName(const std::string& host,
                                                           const std::vector<std::string>&);
    std::pair<MatchType, std::string> MatchedServerName(const http::Request& rq) const;
    // has to call IResponseCallback with rs when the last is rdy
    utils::unique_ptr<AResponseProcessor> ProcessRequest(
        const http::Request& rq, const RequestDestination& rq_dest,
        utils::unique_ptr<http::IResponseCallback> cb) const;
    utils::unique_ptr<AResponseProcessor> GetResponseProcessor(
        const http::Request& rq, const RequestDestination& rq_dest,
        utils::unique_ptr<http::IResponseCallback> cb) const;

    std::string name() const;
    const std::vector<std::string>& server_names() const;
    const std::string& access_log_path() const;
    Severity access_log_level() const;
    const std::string& error_log_path() const;
    const std::vector<utils::shared_ptr<Location> >& locations() const;
    const std::map<int /*response_code*/, std::string /*path_to_err_page*/>& error_pages() const;
    std::string GetDebugString() const;
    std::pair<utils::shared_ptr<Location>, LocationType> ChooseLocation(
        const http::Request& rq) const;

  private:
    std::string access_log_path_;
    Severity access_log_level_;
    std::string error_log_path_;
    std::vector<std::string> server_names_;
    std::map<int /*response_code*/, std::string /*path_to_err_page*/> error_pages_;

    std::vector<utils::shared_ptr<Location> > locations_;
    typedef std::vector<utils::shared_ptr<Location> >::const_iterator LocationsConstIt;

  private:
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
