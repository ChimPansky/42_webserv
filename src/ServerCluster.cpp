#include "ServerCluster.h"
#include "c_api/utils.h"
#include "c_api/EventManager.h"

// testing with one
ServerCluster::ServerCluster(const Config& /*config*/) {
    _servers.push_back(
        utils::unique_ptr<Server>(new Server("Sserv", c_api::IPv4FromString("127.0.0.1"), 8083)));
}

// smth like
void ServerCluster::Run() {
    // register signal for ^C, switch run on that
    while(_run) {
        c_api::EventManager::get().CheckOnce();
        for (ServersIt it = _servers.begin(); it != _servers.end(); ++it) {
            (*it)->CheckClients();
        }
    }
}
