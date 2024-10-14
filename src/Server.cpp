#include "Server.h"
#include "http/Request.h"
#include <iostream>

#include <string>

// create master socket, register read callback for master socket in event manager
Server::Server(const std::string& name)
    : name_(name)
{

}

Server::~Server()
{

}

const std::string& Server::name() const {
    return name_;
}

bool    DoesMatchTheRequest(const std::string& host)
{
    typedef std::vector<std::string>::const_iterator NamesIter;
    //std::string     host = rq.GetHeaderVal("HOST");

    std::vector<std::string>    server_names_;
    server_names_.push_back("server1");
    server_names_.push_back("server2");
    server_names_.push_back("server3");
    server_names_.push_back("*.example.com");
    server_names_.push_back("*.example.*");

    for (NamesIter it = server_names_.begin(); it != server_names_.end(); ++it) {
        std::string    server_name = *it;

        if (host == server_name) {
            return true;
        } else if (server_name[0] == '*' && host.size() >= server_name.size() &&
                 host.compare(host.size() - (server_name.size() - 1), server_name.size() - 1, server_name.substr(1)) == 0) {
            return true;
        } else if (server_name[server_name.size() - 1] == '*' && host.size() >= server_name.size() && server_name.size() > 2 &&
                 host.compare(0, server_name.size() - 2, server_name, 0, server_name.size() - 2) == 0) {
            return true;
        }
    }
    return false;
}

int main() {

    std::string name = std::string("www.example.com");

    std::cout << DoesMatchTheRequest(name) << std::endl;
}
