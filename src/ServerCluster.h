#ifndef WS_SERVER_CLUSTER_H
#define WS_SERVER_CLUSTER_H

#include <vector>

#include "Config.h"
#include "Server.h"
#include "utils/unique_ptr.h"

class ServerCluster {
  public:
    static void Start(const Config& config);
    static void Stop();
  private:
    ServerCluster(const Config&);
    std::vector<utils::unique_ptr<Server> > _servers;
    typedef std::vector<utils::unique_ptr<Server> >::const_iterator ServersIt;
    // check if volatile is necessary for static variables
    static volatile bool _run;
};

#endif  // WS_SERVER_CLUSTER_H
