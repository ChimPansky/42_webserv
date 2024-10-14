#ifndef WS_HTTP_REQUEST_PARSER_H
#define WS_HTTP_REQUEST_PARSER_H

#include <sys/types.h>
#include <vector>
#include <string>

namespace http {

class RequestParser {
  public:
    RequestParser(std::vector<unsigned char> *rq_buf);

    char Peek(ssize_t offset = 0) const;
    bool Advance(ssize_t n = 1);
    void StartNewElement();
    bool EndOfBuffer() const;
    bool ExceededLineLimit() const;
    size_t ElementLen() const;
    std::string ExtractElement(ssize_t end_offset = 0) const;

  private:
    std::vector<unsigned char> *buf_;
    size_t old_buf_size_;
    size_t line_begin_idx_;
    ssize_t element_begin_idx_; // begin of Request-Element, e.g. Method, Uri, Header-Key, Header-Val,...
    ssize_t element_end_idx_;

  public:
    size_t old_buf_size() const;
    size_t element_end_idx() const;
    size_t element_begin_idx() const;

    void set_old_buf_size(size_t sz);
};

} // namespace http

#endif // WS_HTTP_REQUEST_PARSER_H
