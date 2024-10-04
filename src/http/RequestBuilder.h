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
        PS_HEADERS_COMPLETE,
        PS_CHECK_FOR_BODY,
        PS_BODY_REGULAR,
        PS_BODY_CHUNK_SIZE,
        PS_BODY_CHUNK_CONTENT,
        PS_END,
        PS_BAD_REQUEST
    };

  public:
    RequestBuilder();
    bool HasReachedEndOfBuffer() const;
    size_t ParseNext(size_t bytes_read);
    bool IsReadyForResponse();
    bool needs_info_from_server() const;
    void set_max_body_size(size_t max_body_size);
    const Request& rq() const;
    std::vector<char>& buf();

  private:
    Request rq_;
    size_t  chunk_counter_;
    size_t  crlf_counter_;
    std::vector<char>   buf_;
    size_t  begin_idx_;
    size_t  end_idx_;
    ParseState  parse_state_;
    std::string header_key_;
    bool    needs_info_from_server_;

    size_t  ParseLen_() const;
    char    GetNextChar_(void);
    void    NullTerminatorCheck_(char c);
    int     CompareBuf_(const char*, size_t len) const;
    void    UpdateBeginIdx_(void);
    bool    ReadingBody_(void) const;

    ParseState  ParseMethod_(char c);
    ParseState  ParseUri_(char c);
    ParseState  ParseVersion_(void);
    ParseState  CheckForNextHeader_(char c);
    ParseState  ParseHeaderKey_(char c);
    ParseState  ParseHeaderKeyValSep_(char c);
    ParseState  ParseHeaderValue_(char c);
    ParseState  ParseEOF_(void);
    ParseState  AfterHeadersRoutine_(void);
    ParseState  CheckForBody_(void);

    ParseState  ReadBodyRegular_(void);
    ParseState  ReadBodyChunkSize_(char c);

    ParseState  ReadBodyChunkContent_(void);

    bool        DoesBodyExceedMaxSize_() const;

    void PrintParseBuf_() const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
