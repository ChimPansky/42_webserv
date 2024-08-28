#ifndef WS_LOCATIONBLOCK_H
#define WS_LOCATIONBLOCK_H

#include "HttpBlock.h"
#include <vector>

class   LocationBlock : public HttpBlock {

  private:
    std::vector<std::string> _allow_methods;
    std::pair<int, std::string> _return;
    bool _cgi;
    std::string _cgi_path;
    std::string _cgi_extension;

  public:
    LocationBlock(const HttpBlock&);
    const std::vector<std::string>& allow_methods() const;
    const std::pair<int, std::string>& return_code() const;
    bool cgi() const;
    const std::string& cgi_path() const;
    const std::string& cgi_extension() const;
    virtual void parse(std::ifstream& file);
    virtual const std::vector<std::string>&    getTokens();
};

#endif  // WS_LOCATIONBLOCK_H
