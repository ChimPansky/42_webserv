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
        PS_METHOD,
        PS_URI,
        PS_VERSION,
        PS_HEADER_KEY,
        PS_HEADER_VALUE,
        PS_BODY,
        PS_END,
        PS_ERROR
    };
  public:
    RequestBuilder();
    void Reset();
    void ParseNext(const char* input, size_t input_sz);
    bool is_ready_for_response() const;
    const Request& rq() const;

  private:
    Request rq_;
    //Server& server_;
    bool eof_reached_;

    std::vector<char> parse_buf_;
    int chunk_counter_;
    ParseState parse_state_;
    size_t parse_buf_len_;
    std::string header_key_;
    ParseState ParseMethod_();
    ParseState ParseUri_();
    ParseState ParseVersion_();
    ParseState ParseHeaderKey_();
    ParseState ParseHeaderValue_();
    ParseState ParseBody_();

    void CheckIfRequestIsComplete_();
    void ResetParseBuf_();
    bool LineIsEmpty_() const;

    void PrintParseBuf_() const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
