// maybe we dont even need this class. --> do the parsing directly in Request class...
#ifndef WS_REQUEST_PARSER_H
#define WS_REQUEST_PARSER_H

#include <string>
namespace http {

class Request;

class RequestParser {
  private:
    enum ParseState {
        PS_START,
        PS_METHOD,
        PS_URI,
        PS_VERSION,
        PS_HEADERS,
        PS_BODY,
        PS_END,
        PS_ERROR
    };

  public:
    RequestParser(Request& rq);

    void ParseNext(const char* chunk, size_t chunk_sz);
    void ParseMethod();
    void ParseURI();
    void ParseVersion();
    void ParseHeaders();
    void ParseBody();

    void Print() const;

  private:
    Request& _rq;
    ParseState _parse_state;
    std::string _parse_str;
    size_t _parse_idx;
};

}  // namespace http

#endif  // WS_REQUEST_PARSER_H
