#include "ServerCluster.h"

#include "c_api/EventManager.h"
#include "c_api/utils.h"

// testing with one
ServerCluster::ServerCluster(const Config& /*config*/)
{
    servers_.push_back(
        utils::unique_ptr<Server>(new Server("Sserv", c_api::IPv4FromString("127.0.0.1"), 8080)));
}

volatile bool ServerCluster::run_ = false;

void ServerCluster::Stop()
{
    run_ = false;
}

// smth like
void ServerCluster::Start(const Config& config)
{
    // register signal for ^C, switch run on that
    run_ = true;
    ServerCluster cluster(config);
    while (run_) {
        c_api::EventManager::get().CheckOnce();
        for (ServersIt it = cluster.servers_.begin(); it != cluster.servers_.end(); ++it) {
            (*it)->CheckClients();
        }
    }
}
