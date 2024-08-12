#include "Webserver.h"

Webserver::Webserver(in_addr_t ip, in_port_t port)
  : _master_sock(ip, port)
{

}


void Webserver::Run() {
    // wait for connection
    // get connection
    // read from connection
    // process doc
    // write to connection
}
