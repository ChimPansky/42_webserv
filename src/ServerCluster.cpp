#include "ServerCluster.h"
#include "c_api/utils.h"

// testing with one
ServerCluster::ServerCluster(const Config& config) {
    _servers.push_back(
        std::auto_ptr(new Server(c_api::ipv4_from_string("127.0.0.1"), 8080, _event_manager)));
}

// smth like
void ServerCluster::Run() {
    // register signal for ^C, switch run on that
    while(_run) {
        _event_manager.check_once();
        for (auto serv : _servers) {
            serv->check_clients();
        }
    }
}
