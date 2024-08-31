#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include "Request.h"
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
    void ParseChunk(const char* chunk, size_t chunk_size);
    bool IsRequestReady() const;
    const Request& rq() const;

  private:
    int chunk_counter_;
    Request rq_;
    ParseState parse_state_;
    //std::string parse_buf_;
   // size_t parse_buf_idx_;
    void ParseMethod();
    void ParseUri();
    void ParseVersion();
    void ParseHeaders();
    void ParseBody();
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
