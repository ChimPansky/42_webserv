#include "RequestParser.h"
#include "http.h"

#include <cstring>
#include <logger.h>
#include <sstream>


namespace http {

RequestParser::RequestParser()
    : old_buf_size_(0), element_end_idx_(0)
{}

std::vector<char>& RequestParser::buf()
{
    return buf_;
}

void RequestParser::PrepareToRecvData(size_t recv_size)
{
    old_buf_size_ = buf_.size();
    buf_.resize(buf_.size() + recv_size);
}

void RequestParser::AdjustBufferSize(size_t bytes_recvd)
{
    if (buf_.size() > old_buf_size_ + bytes_recvd) {
        buf_.resize(old_buf_size_ + bytes_recvd);
    }
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

size_t RequestParser::RemainingLength() const {
    return buf_.size() - element_end_idx_;
}

bool RequestParser::ExceededLineLimit() const
{
    return element_end_idx_ > RQ_LINE_LEN_LIMIT;
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
    LOG(DEBUG) << "erasing " << end << " bytes";
    buf_.erase(buf_.begin(), buf_.begin() + end);
    element_end_idx_ = 0;
    return element;
}

std::string RequestParser::ExtractLine()
{
    size_t len_without_crlf =
        (element_end_idx_ <= 0 ? 0 : element_end_idx_ - 1);
    std::string line = std::string(buf_.data(), buf_.data() + len_without_crlf);
    LOG(DEBUG) << "erasing " << len_without_crlf + 1 << " bytes";
    buf_.erase(buf_.begin(), buf_.begin() + len_without_crlf + 2);
    element_end_idx_ = 0;
    return line;
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

bool RequestParser::FoundCRLF() const {
    if (element_end_idx_ < 1) {
        return false;
    }
    return buf_[element_end_idx_ - 1] == '\r' && buf_[element_end_idx_] == '\n';
}

bool RequestParser::FoundSingleCR() const {
    if (element_end_idx_ < 1) {
        return false;
    }
    return buf_[element_end_idx_ - 1] == '\r' && buf_[element_end_idx_] != '\n';
}

}  // namespace http
