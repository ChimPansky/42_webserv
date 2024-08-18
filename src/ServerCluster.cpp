#include "ServerCluster.h"
#include "c_api/utils.h"
#include "c_api/EventManager.h"

// testing with one
ServerCluster::ServerCluster(const Config& /*config*/)
{
    _servers.push_back(
        utils::unique_ptr<Server>(new Server("Sserv", c_api::ipv4_from_string("127.0.0.1"), 8083)));
}

volatile bool ServerCluster::_run = false;

void ServerCluster::Stop() {
    _run = false;
}

// smth like
void ServerCluster::Start(const Config& config) {
    // register signal for ^C, switch run on that
    _run = true;
    ServerCluster cluster(config);
    while(_run) {
        c_api::EventManager::get().check_once();
        for (ServersIt it = cluster._servers.begin(); it != cluster._servers.end(); ++it) {
            (*it)->check_clients();
        }
    }
}
