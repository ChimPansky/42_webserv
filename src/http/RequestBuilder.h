#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include <cstddef>
#include <vector>

#include "Request.h"

class Server;
namespace http {

class RequestBuilder {
  private:
    enum ParseState {
        PS_METHOD,
        PS_URI,
        PS_VERSION,
        PS_BETWEEN_HEADERS,
        PS_HEADER_KEY,
        PS_HEADER_KEY_VAL_SEP,
        PS_HEADER_VALUE,
        PS_AFTER_HEADERS,
        PS_BODY,
        PS_END,
        PS_BAD_REQUEST
    };

  public:
    RequestBuilder();
    void ParseNext(void);
    bool IsReadyForResponse();
    const Request& rq() const;
    std::vector<char>& buf();

  private:
    Request rq_;
    size_t chunk_counter_;
    size_t crlf_counter_;
    std::vector<char> buf_;
    size_t begin_idx_;
    size_t end_idx_;
    ParseState parse_state_;
    std::string header_key_;

    size_t  ParseLen_() const;
    void    NullTerminatorCheck_(char c);
    int     CompareBuf_(const char*, size_t len) const;
    void    UpdateBeginIdx_();

    ParseState ParseMethod_(char c);
    ParseState ParseUri_(char c);
    ParseState ParseVersion_(char c);
    ParseState CheckForNextHeader_(char c);
    ParseState ParseHeaderKey_(char c);
    ParseState ParseHeaderKeyValSep_(char c);
    ParseState ParseHeaderValue_(char c);
    ParseState ParseBody_(char c);
    ParseState ParseEOF_(void);
    ParseState CheckForBody_(void);

    void PrintParseBuf_() const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
