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

  private:
    std::string name_;
};

#endif  // WS_SERVER_H
