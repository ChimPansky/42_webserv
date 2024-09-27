#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include <cstddef>
#include <vector>

#include "Request.h"

class Server;
namespace http {

class RequestBuilder {
  private:
    struct EOFChecker {
      private:
        short int counter_;

      public:
        EOFChecker();
        void Update(char c);
        bool end_of_line_;
        bool end_of_file_;
    };

  private:
    enum ParseState {
        PS_METHOD,
        PS_URI,
        PS_VERSION,
        PS_HEADER_KEY,
        PS_HEADER_SEP,
        PS_HEADER_VALUE,
        PS_BODY,
        PS_END,
        PS_ERROR
    };

  public:
    RequestBuilder();
    void ParseNext(void);
    bool IsReadyForResponse();
    const Request& rq() const;
    std::vector<char>& buf();

  private:
    Request rq_;
    EOFChecker eof_checker_;
    // Server& server_;
    size_t chunk_counter_;
    std::vector<char> buf_;
    size_t begin_idx_;
    size_t end_idx_;
    ParseState parse_state_;
    std::string header_key_;

    ParseState ParseMethod_(char c);
    ParseState ParseUri_(char c);
    ParseState ParseVersion_(char c);
    ParseState ParseHeaderKey_(char c);
    ParseState ParseHeaderSep_(char c);
    ParseState ParseHeaderValue_(char c);
    ParseState ParseBody_(char c);

    size_t ParseLen_() const;
    void UpdateBeginIdx_();
    bool LineIsEmpty_() const;

    void PrintParseBuf_() const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
