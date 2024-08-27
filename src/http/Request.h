#ifndef WS_HTTP_REQUEST_H
#define WS_HTTP_REQUEST_H

#include <cstddef>
#include <map>
#include <string>

#include "RequestParser.h"
namespace http {
enum Method {
    GET,
    POST,
    DELETE
};
enum Version {  // probably only need to handle Ver_1_0 and Ver_1_1
    Ver0_9,
    Ver1_0,
    Ver1_1,
    Ver2,
    Ver3
};

extern const char* httpEOF;

class RequestParser;

class Request {
  public:
    Request();
    void AddChunkToRequest(const char* chunk, size_t chunk_sz);

    // Getters:
    const std::string& raw_request() const;
    Method method() const;
    Version& version() const;

    // Setters:
    void set_method(Method);
    void set_version(Version);
    void set_uri(const std::string& uri);

    void Print() const;

  private:
    RequestParser _rq_parser;
    std::string _raw_request;
    Method _method;
    Version _version;
    std::string _uri;  // later: put this in struct/class with path, query string (?). fragment (#)
    std::string _host;
    std::string _user_agent;
    std::string _accept;
    std::map<std::string, std::string> _headers;
    size_t _body_len;
    std::string _body;
    int _error;
};
}  // namespace http

#endif  // WS_HTTP_REQUEST_H
