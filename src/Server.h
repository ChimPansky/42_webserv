#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <netinet/in.h>

#include <map>
#include <string>

#include "ClientSession.h"
#include "c_api/MasterSocket.h"
#include "utils/unique_ptr.h"

class Server {
  private:
    Server();
    Server(const Server&);
    Server& operator=(const Server&);

  public:
    // create master socket, register read callback for master socket in event manager
    Server(const std::string& name);
    ~Server();

    const std::string& name() const;

    // chimpansky start
    // when a client sends a (valid) request, iterate through ServerCluster::servers_ and call Compare() on each server.
    // if Compare() returns true, call ProcessRequest() on that server (this will return a http::Response) and send the Response back to the client (using ClientSession::ClientWriteCallback::Call()).
    // Compare examples:
    // Compare("127.0.0.1:8081", "/")
    // Compare("localhost", "path/to/file")
    // Compare("localhost", "path/to/file?query=string")
    // Question: location is full URI or just path portion of URI?
    bool Compare(const std::string& host, const std::string& location) const;
    // Response examples:
    // http::Response response = server.ProcessRequest(request);
    http::Response ProcessRequest(const http::Request& request);
    // chimpansky end

  private:
    std::string name_;
};

#endif  // WS_SERVER_H
