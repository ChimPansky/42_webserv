#ifndef WS_HTTP_REQUEST_PARSER_H
#define WS_HTTP_REQUEST_PARSER_H

#include <sys/types.h>

#include <string>
#include <vector>

namespace http {

class RequestParser {
  public:
    RequestParser();

    std::vector<char>& buf();
    void AddNewData(const char* data, size_t data_sz);

    char Peek(ssize_t offset = 0) const;
    bool Advance(ssize_t n = 1);
    bool FoundCRLF() const;
    bool FoundSingleCR() const;
    void StartNewElement();
    bool EndOfBuffer() const;
    bool ExceededLineLimit() const;
    size_t ElementLen() const;
    size_t RemainingLength() const;
    std::string ExtractElement();
    std::string ExtractLine();

    char& operator[](ssize_t index);

  private:
    std::vector<char> buf_;
    size_t old_buf_size_;
    ssize_t element_end_idx_;

  public:
    size_t element_end_idx() const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_PARSER_H
