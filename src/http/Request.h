#ifndef WS_HTTP_REQUEST_H
#define WS_HTTP_REQUEST_H

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

class Request {
  public:
    void AddChunkToRequest(const char* chunk, size_t chunk_sz);
    const std::string& raw_request() const;

  private:
    std::string _raw_request;
    // http::Method _method;
    std::string _url;  // later: put this in struct/class with path, query string (?). fragment (#)
    // http::Version _version;
    std::map<std::string, std::string> _headers;
    std::string _body;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_H
