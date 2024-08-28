#ifndef WS_SERVERBLOCK_H
#define WS_SERVERBLOCK_H

#include <fstream>
#include <list>
#include <string>

#include "LocationBlock.h"

class   ServerBlock : public HttpBlock {

  private:
    ServerBlock();
    std::string _access_log;
    std::string _access_log_level;
    int _listen;
    std::vector<std::string> _server_names;
    std::list<std::pair<std::string, LocationBlock> > _locations;

  public:
    ServerBlock(const HttpBlock& http_block);
    const std::string&  access_log() const;
    const std::string&  access_log_level() const;
    int listen() const;
    const std::vector<std::string>& server_names();
    const std::list<std::pair<std::string, LocationBlock> >& locations();
    void parse(std::ifstream& file);
    const std::vector<std::string>&    getTokens();
};

#endif  // WS_SERVERBLOCK_H
