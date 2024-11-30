#include "RequestParser.h"
#include "http.h"

#include <cstring>


namespace http {

RequestParser::RequestParser()
    : old_buf_size_(0), line_begin_idx_(0), element_begin_idx_(0), element_end_idx_(0)
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

int RequestParser::CompareBuf_(const char* str, size_t len) const
{
    return std::strncmp((buf_.data()) + element_begin_idx(), str, len);
}

size_t RequestParser::ElementLen() const
{
    return element_end_idx_ - element_begin_idx_ + 1;
}

bool RequestParser::ExceededLineLimit() const
{
    return line_begin_idx_ > RQ_LINE_LEN_LIMIT;
}

void RequestParser::StartNewElement()
{
    element_begin_idx_ = element_end_idx_;
}

bool RequestParser::EndOfBuffer() const
{
    return element_end_idx_ >= static_cast<ssize_t>(buf_.size()) - 1;
}

std::string RequestParser::ExtractElement(ssize_t end_offset) const
{
    if (element_end_idx_ + end_offset < element_begin_idx_) {
        return "";
    }
    if (element_end_idx_ + end_offset < 2) {
        return "";
    }
    size_t begin = element_begin_idx_;
    size_t end =
        std::max(static_cast<ssize_t>(element_end_idx_) + end_offset, static_cast<ssize_t>(begin));
    end = std::min(end, buf_.size());
    // LOG(DEBUG) << "{" << std::string(buf_-> data() + begin, buf_->data() + end) << "}";
    return std::string(buf_.data() + begin, buf_.data() + end);
}

std::string RequestParser::ExtractLine() const
{
    size_t begin = element_begin_idx_;
    size_t end =
        std::max(static_cast<ssize_t>(element_end_idx_), static_cast<ssize_t>(begin));
    end = std::min(end, buf_.size());
    return std::string(buf_.data() + begin, buf_.data() + end - 1);
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

size_t RequestParser::old_buf_size() const
{
    return old_buf_size_;
}

size_t RequestParser::element_end_idx() const
{
    return element_end_idx_;
}

size_t RequestParser::element_begin_idx() const
{
    return element_begin_idx_;
}

void RequestParser::set_old_buf_size(size_t sz)
{
    old_buf_size_ = sz;
}

}  // namespace http
