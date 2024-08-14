#ifndef WS_SERVER_CLUSTER_H
#define WS_SERVER_CLUSTER_H

#include <memory>
#include <vector>

#include "Config.h"
#include "Server.h"
#include "utils/ICallback.h"

class ServerCluster {
  public:
    ServerCluster(const Config&);
    void Run();
  private:
    std::vector<std::auto_ptr<Server> > _servers;
    c_api::EventManager _event_manager;
    bool _run;
};

#endif  // WS_SERVER_CLUSTER_H
