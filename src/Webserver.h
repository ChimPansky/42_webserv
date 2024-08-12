#ifndef WS_C_API_WEBSERVER_H
#define WS_C_API_WEBSERVER_H

#include "c_api/MasterSocket.h"
#include "c_api/SlaveSocket.h"

#include <netinet/in.h>

#include <memory>
#include <set>

class Webserver {
  private:
    Webserver();
    Webserver(const Webserver&);
    Webserver& operator=(const Webserver&);
  public:
    Webserver(in_addr_t ip, in_port_t port);
    void Run();  // noreturn?
  private:
    c_api::MasterSocket _master_sock;
    std::set<std::auto_ptr<c_api::SlaveSocket>> _slave_sockets;
};

#endif  // WS_C_API_WEBSERVER_H
