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
    // only if hostname of the request matches any of the server names
    bool DoesMatchTheRequest(const http::Request& rq) const;

  private:
    std::string name_;
    std::vector<std::string> server_names_;
};

#endif  // WS_SERVER_H
