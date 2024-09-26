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
        void Reset();
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
    void Reset();
    void ParseNext(const char* input, size_t input_sz);
    bool IsReadyForResponse();
    const Request& rq() const;

  private:
    Request rq_;
    EOFChecker eof_checker_;
    // Server& server_;
    std::vector<char> parse_buf_;
    int chunk_counter_;
    ParseState parse_state_;
    std::string header_key_;

    ParseState ParseMethod_();
    ParseState ParseUri_();
    ParseState ParseVersion_();
    ParseState ParseHeaderKey_();
    ParseState ParseHeaderSep_();
    ParseState ParseHeaderValue_();
    ParseState ParseBody_();

    void ResetParseBuf_();
    bool LineIsEmpty_() const;

    void PrintParseBuf_() const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
