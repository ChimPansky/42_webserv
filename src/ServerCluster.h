#ifndef WS_SERVER_CLUSTER_H
#define WS_SERVER_CLUSTER_H

#include <vector>

#include "Config.h"

class ServerCluster {
  public:
    ServerCluster(const Config&);
    void Run();
  private:
    bool _run;
};

#endif  // WS_SERVER_CLUSTER_H
