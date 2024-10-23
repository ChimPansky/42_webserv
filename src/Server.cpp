#include "Server.h"

#include <iostream>
#include <string>

#include "http/Request.h"

// create master socket, register read callback for master socket in event manager
Server::Server(const std::string& name) : name_(name)
{}

Server::~Server()
{}

const std::string& Server::name() const
{
    return name_;
}

bool DoesMatchTheRequest(const std::string& host)
{
    typedef std::vector<std::string>::const_iterator NamesIter;
    // std::string     host = rq.GetHeaderVal("HOST");

    std::vector<std::string> server_names_;
    server_names_.push_back("server1");
    server_names_.push_back("server2");
    server_names_.push_back("server3");
    server_names_.push_back("*.example.com");
    server_names_.push_back("*.example.*");

    for (NamesIter it = server_names_.begin(); it != server_names_.end(); ++it) {
        std::string server_name = *it;

        if (host == server_name) {
            return true;
        } else if (server_name[0] == '*' && host.size() >= server_name.size() &&
                   host.compare(host.size() - (server_name.size() - 1), server_name.size() - 1,
                                server_name.substr(1)) == 0) {
            return true;
        } else if (server_name[server_name.size() - 1] == '*' &&
                   host.size() >= server_name.size() && server_name.size() > 2 &&
                   host.compare(0, server_name.size() - 1, server_name, 0,
                                server_name.size() - 1) == 0) {
            return true;
        }
    }
    return false;
}

// void TestMatch(const std::string& host) {
//     if (DoesMatchTheRequest(host)) {
//         std::cout << "Matches: " << host << std::endl;
//     } else {
//         std::cout << "Doesn't match: " << host << std::endl;
//     }
// }

// int main() {

//     std::cout << "Match:" << std::endl;
//     TestMatch("server1");  // should match
//     TestMatch("server2");  // should match
//     TestMatch("server3");  // should match

//     TestMatch("sub.sub.example.com");  // should match

//     // prefix match
//     TestMatch("www.example.com");  // should match
//     TestMatch("shop.example.com"); // should match
//     std::cout << std::endl << "Don't match:" << std::endl;
//     TestMatch("example.com");      // shouldn't match

//     TestMatch("server123");   // shouldn't match
//     TestMatch("serverx");     // shouldn't match
//     TestMatch("serversuffix");// shouldn't match

//     TestMatch("randomhost");  // shouldn't match
//     TestMatch("example.org"); // shouldn't match

//     TestMatch("");  // shouldn't match
//     TestMatch("wwwexample.com");  // shouldn't match
//     TestMatch("www.examplecom");  // shouldn't match

//     TestMatch("example.co");       // shouldn't match
//     TestMatch("sub.example.co");   // shouldn't match
// }
