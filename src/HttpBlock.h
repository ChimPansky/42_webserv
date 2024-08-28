#ifndef WS_HTTPBLOCK_H
#define WS_HTTPBLOCK_H

#include <fstream>
#include <map>
#include <string>
#include <vector>

class HttpBlock {

  protected:
    std::string _root;
    std::string _index;
    bool _autoindex;
    int _client_max_body_size;
    int _keepalive_timeout;
    std::map<int, std::string> _error_pages;

  public:
    HttpBlock();
    HttpBlock(const HttpBlock& copy);
    HttpBlock& operator=(const HttpBlock& copy);
    virtual ~HttpBlock();
    const std::string& root() const;
    const std::string& index() const;
    bool autoindex() const;
    int client_max_body_size() const;
    int keepalive_timeout() const;
    const std::map<int, std::string>& error_pages();
    virtual void  parse( std::ifstream& file );
    virtual const std::vector<std::string>&    getTokens();
};

#endif  // WS__HTTPBLOCK_H
