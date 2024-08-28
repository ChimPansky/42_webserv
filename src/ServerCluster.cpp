#include "ServerCluster.h"

#include "c_api/EventManager.h"
#include "c_api/utils.h"

// testing with one
ServerCluster::ServerCluster(const Config& /*config*/)
{
    _servers.push_back(
        utils::unique_ptr<Server>(new Server("Sserv", c_api::IPv4FromString("127.0.0.1"), 8080)));
}

volatile bool ServerCluster::_run = false;

void ServerCluster::Stop()
{
    _run = false;
}

// smth like
void ServerCluster::Start(const Config& config)
{
    // register signal for ^C, switch run on that
    _run = true;
    ServerCluster cluster(config);
    while (_run) {
        c_api::EventManager::get().CheckOnce();
        for (ServersIt it = cluster._servers.begin(); it != cluster._servers.end(); ++it) {
            (*it)->CheckClients();
        }
    }
}
