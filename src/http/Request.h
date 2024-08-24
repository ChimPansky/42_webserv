#ifndef WS_HTTP_REQUEST_H
#define WS_HTTP_REQUEST_H

#include <cstddef>
#include <map>
#include <string>
namespace http {
enum Method {
    GET,
    POST,
    DELETE
};
enum Version {
    Ver0_9,
    Ver1_0,
    Ver1_1,
    Ver2,
    Ver3
};

static const char* httpEOF = "\r\n\r\n";

class Request {
  public:
    void                AddChunkToRequest(const char* chunk, size_t chunk_sz);
    void                ParseNext();
    void                ParseMethod();
    void                ParseURI();
    void                ParseVersion();
    void                ParseHeaders();
    void                ParseBody();
    const std::string&  raw_request() const;
    void                Print() const;

  private:
    enum ParseState {
        kStart,
        kMethod,
        kURI,
        kVersion,
        kHeaders,
        kBody,
        kEnd
    };
    ParseState  _parse_state;
    std::string _parse_str;
    size_t      _parse_idx;
    std::string _raw_request;
    // http::Method _method;
    std::string _url;  // later: put this in struct/class with path, query string (?). fragment (#)
    // http::Version _version;
    std::map<std::string, std::string> _headers;
    std::string _body;
    int _error;

};
}  // namespace http

#endif  // WS_HTTP_REQUEST_H
