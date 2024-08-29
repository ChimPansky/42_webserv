#ifndef WS_SERVERBLOCK_H
#define WS_SERVERBLOCK_H

#include <list>

#include "LocationBlock.h"

#include <string>

class   ServerBlock {

  private:
    std::string access_log_;
    std::string access_log_level_;
    int port_;
    std::string root_dir_;
    std::string default_file_;
    bool dir_listing_;
    int client_max_body_size_;
    std::map<int, std::string> error_pages_;
    std::vector<std::string> server_names_;
    std::list<std::pair<std::string, LocationBlock> > locations_;

  public:
    ServerBlock();
    const std::string&  access_log() const;
    const std::string&  access_log_level() const;
    int port() const;
    const std::string&  root_dir() const;
    const std::string&  default_file() const;
    bool dir_listing() const;
    int client_max_body_size() const;
    const std::map<int, std::string>& error_pages() const;
    const std::vector<std::string>& server_names();
    const std::list<std::pair<std::string, LocationBlock> >& locations();
    static const std::vector<std::string>    GetTokens();
    void  set_access_log(const std::string& access_log);
    void  set_access_log_level(const std::string& access_log_level);
    void  set_port(int port);
    void  set_root_dir(const std::string& root_dir);
    void  set_default_file(const std::string& default_file);
    void  set_dir_listing(bool dir_listing);
    void  set_client_max_body_size(int client_max_body_size);
    void  set_error_pages(const std::map<int, std::string>& error_pages);
};

#endif  // WS_SERVERBLOCK_H
