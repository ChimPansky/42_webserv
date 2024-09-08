#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include "Request.h"
#include <cstddef>
#include <vector>

class Server;
namespace http {

class RequestBuilder {
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
    RequestBuilder();
    void Reset();
    void ParseNext(const std::vector<char>& buf);
    bool is_ready_for_response() const;
    const Request& rq() const;

  private:
    Request rq_;
    //Server& server_;
    bool eof_reached_;
    int chunk_counter_;
    ParseState parse_state_;
    size_t parse_idx_;
    void ParseMethod();
    void ParseUri();
    void ParseVersion();
    void ParseHeaders();
    void ParseBody();
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
