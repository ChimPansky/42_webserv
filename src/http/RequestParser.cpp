#include "RequestParser.h"

#include <logger.h>
#include <iterator>

#include "http.h"


namespace http {

RequestParser::RequestParser() : element_end_idx_(0)
{}

std::vector<char>& RequestParser::buf()
{
    return buf_;
}

void RequestParser::AddNewData(const char* data, size_t data_sz)
{
    buf_.reserve(buf_.size() + data_sz);
    std::copy(data, data + data_sz, std::back_inserter(buf_));
}

bool RequestParser::Advance(ssize_t n)
{
    element_end_idx_ += n;
    return true;
}

char RequestParser::Peek(ssize_t offset) const
{
    if (element_end_idx_ + offset < 0 ||
        element_end_idx_ + offset >= static_cast<ssize_t>(buf_.size())) {
        return '\0';
    }
    return buf_[element_end_idx_ + offset];
}

size_t RequestParser::ElementLen() const
{
    return element_end_idx_ + 1;
}

size_t RequestParser::RemainingLength() const
{
    return buf_.size() - element_end_idx_;
}


bool RequestParser::EndOfBuffer() const
{
    return element_end_idx_ >= static_cast<ssize_t>(buf_.size());
}

std::string RequestParser::ExtractElement()
{
    size_t end = (element_end_idx_ >= 0 ? element_end_idx_ : 0);
    end = std::min(end, buf_.size());
    std::string element = std::string(buf_.data(), buf_.data() + end);
    buf_.erase(buf_.begin(), buf_.begin() + end);
    element_end_idx_ = 0;
    return element;
}

std::string RequestParser::ExtractLine()
{
    size_t len_without_crlf = (element_end_idx_ <= 0 ? 0 : element_end_idx_ - 1);
    std::string line = std::string(buf_.data(), buf_.data() + len_without_crlf);
    // LOG(DEBUG) << "erasing " << len_without_crlf + 1 << " bytes";
    buf_.erase(buf_.begin(), buf_.begin() + len_without_crlf + 2);
    element_end_idx_ = 0;
    return line;
}

// move n bytes from RequestParser-buffer to destiation stream (deleting n bytes from buffer)
size_t RequestParser::MoveToStream(size_t n, std::ostream& dest)
{
    if (n > buf_.size()) {
        n = buf_.size();
    }
    std::copy(buf_.begin(), buf_.begin() + n, std::ostream_iterator<char>(dest));
    buf_.erase(buf_.begin(), buf_.begin() + n);
    element_end_idx_ = 0;
    return n;
}

char& RequestParser::operator[](ssize_t index)
{
    if (index < 0) {
        return buf_[0];
    }
    if (index > static_cast<ssize_t>(buf_.size())) {
        return buf_[buf_.size() - 1];
    }
    return buf_[index];
}

size_t RequestParser::element_end_idx() const
{
    return element_end_idx_;
}

bool RequestParser::EndsWithCRLF() const
{
    if (element_end_idx_ < 1) {
        return false;
    }
    return buf_[element_end_idx_ - 1] == '\r' && buf_[element_end_idx_] == '\n';
}

bool RequestParser::BeginsWithCRLF() const
{
    if (buf_.size() < 2) {
        return false;
    }
    return buf_[0] == '\r' && buf_[1] == '\n';
}

void RequestParser::Ignore(size_t n)
{
    if (n > buf_.size()) {
        n = buf_.size();
    }
    if (n <= 0) {
        return;
    }
    buf_.erase(buf_.begin(), buf_.begin() + n);
    element_end_idx_ = 0;
}

bool RequestParser::FoundSingleCR() const
{
    if (element_end_idx_ < 1) {
        return false;
    }
    return buf_[element_end_idx_ - 1] == '\r' && buf_[element_end_idx_] != '\n';
}

}  // namespace http
