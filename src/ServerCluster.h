#ifndef WS_SERVER_CLUSTER_H
#define WS_SERVER_CLUSTER_H

#include <vector>

#include "Config.h"
#include "Server.h"
#include "utils/unique_ptr.h"

class ServerCluster {
  public:
    ServerCluster(const Config&);
    void Run();
  private:
    std::vector<utils::unique_ptr<Server> > _servers;
    typedef std::vector<utils::unique_ptr<Server> >::const_iterator ServersIt;
    bool _run;
};

#endif  // WS_SERVER_CLUSTER_H
