#ifndef WS_HTTP_REQUEST_PARSER_H
#define WS_HTTP_REQUEST_PARSER_H

#include <sys/types.h>

#include <string>
#include <vector>

namespace http {

class RequestParser {
  public:
    RequestParser();

    std::vector<unsigned char>& buf();
    void ExpandBuf(size_t recv_size);
    void ShrinkBuf(size_t bytes_recvd);

    char Peek(ssize_t offset = 0) const;
    int CompareBuf_(const char*, size_t len) const;
    bool Advance(ssize_t n = 1);
    void StartNewElement();
    bool EndOfBuffer() const;
    bool ExceededLineLimit() const;
    size_t ElementLen() const;
    std::string ExtractElement(ssize_t end_offset = 0) const;

    unsigned char& operator[](ssize_t index);

  private:

    std::vector<unsigned char> buf_;
    size_t old_buf_size_;
    size_t line_begin_idx_;
    ssize_t element_begin_idx_;  // begin of Request-Element, e.g. Method, Uri, Header-Key,
                                 // Header-Val,...
    ssize_t element_end_idx_;

  public:
    size_t old_buf_size() const;
    size_t element_end_idx() const;
    size_t element_begin_idx() const;

    void set_old_buf_size(size_t sz);
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_PARSER_H
