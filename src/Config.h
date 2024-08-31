#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include <utils/unique_ptr.h>
#include <netinet/in.h>
//#include "conf_pars.h"
#include "ServerBlock.h"

#include <string>
#include <vector>

// hopefully will be trivially-copyable
class Config {

  private:
    Config();
    std::string mx_type_;
    std::string error_log_path_;
    std::string error_log_level_;
    int   keepalive_timeout_;
    size_t   client_max_body_size_;
    std::map</* status code */int, /* error page path */std::string> error_pages_;
    std::vector<std::pair<in_addr_t, in_port_t> > listeners_;
    std::vector<utils::unique_ptr<ServerBlock> > server_configs_;

  public:
    Config(const char* config_path);
    const std::string&  mx_type() const;
    const std::string&  error_log_path();
    const std::string&  error_log_level() const;
    int   keepalive_timeout() const;
    size_t   client_max_body_size() const;
    const std::map<int, std::string>& error_pages() const;
    const std::vector<std::pair<in_addr_t, in_port_t> > listeners() const;
    const std::vector<utils::unique_ptr<ServerBlock> >& server_configs() const;
    utils::unique_ptr<ServerBlock>  FindServerBlock(std::pair<in_addr_t, in_port_t>, const std::string& server_name);
    static const std::vector<std::string>    GetTokens();
};

#endif  // WS_CONFIG_H
