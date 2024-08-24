// maybe we dont even need this class. --> do the parsing directly in Request class...
#ifndef WS_REQUEST_PARSER_H
#define WS_REQUEST_PARSER_H

#include <map>
#include <string>

namespace http {

class RequestParser {
  public:
  private:
    std::map<std::string, std::string> _headers;
};

}  // namespace http

#endif  // WS_REQUEST_PARSER_H
