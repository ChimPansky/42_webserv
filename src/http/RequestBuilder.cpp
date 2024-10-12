#include "RequestBuilder.h"

#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#include "Request.h"
#include "utils/logger.h"
#include "utils/utils.h"

namespace http {

RequestBuilder::RequestBuilder()
    : builder_status_(RB_BUILDING), parser_(&buf_), build_state_(BS_METHOD), body_builder_(&rq_.body)
{}

RequestBuilder::Parser::Parser(std::vector<char> *rq_buf)
    : buf(rq_buf), element_begin_idx(0), end_idx(0), remaining_length(0)
{}

RequestBuilder::BodyBuilder::BodyBuilder(std::vector<char> *rq_body)
    : body(rq_body), chunked(false), body_idx(0), remaining_length(0), max_body_size(0)
{}

void RequestBuilder::Parser::Advance(ssize_t n) {
    end_idx += n;
    remaining_length -= n;
}

char RequestBuilder::Parser::Peek() const {
    return (*buf)[end_idx];
}

size_t RequestBuilder::Parser::ElementLen() const {
    return end_idx - element_begin_idx;
}

void RequestBuilder::Parser::UpdateBeginIdx() {
    element_begin_idx = end_idx;
    LOG(DEBUG) << "Parser::UpdateBeginIdx()";
    ExtractElementEOL();
    ExtractElementNoEOL();

}

bool RequestBuilder::Parser::HasReachedEnd() const {
    LOG(DEBUG) << "Parser::HasReachedEnd()? " << (remaining_length == 0 ? "Yes" : "No");
    return remaining_length == 0;
}

std::string RequestBuilder::Parser::ExtractElementEOL() const
{
    size_t begin = element_begin_idx;
    size_t end = end_idx - 2;
    LOG(DEBUG) << "ExtractElementEOL() element_begin_idx: " << begin << "; end_idx - 2: " << end;
    LOG(DEBUG) << "{" << std::string(buf-> data() + std::max(begin, (size_t)0), buf->data() + std::max(begin, end)) << "}";
    return std::string(buf-> data() + std::max(begin, (size_t)0), buf->data() + std::max(begin, end));
}

std::string RequestBuilder::Parser::ExtractElementNoEOL() const
{

    size_t begin = element_begin_idx;
    size_t end = end_idx;
    LOG(DEBUG) << "ExtractElementNoEOL() element_begin_idx: " << begin << "; end_idx: " << end;
    LOG(DEBUG) << "{" << std::string(buf-> data() + std::max(begin, (size_t)0), buf->data() + std::max(begin, end)) << "}";
    return std::string(buf-> data() + std::max(begin, (size_t)0), buf->data() + std::max(begin, end));
}

void RequestBuilder::PrepareToRecvData(size_t recv_size) {
    LOG(DEBUG) << "Builder::PrepareToRecvData (" << recv_size << ")";
    LOG(DEBUG) << "buf_resize: " << buf_.size() << " -> " << buf_.size() + recv_size;
    buf_.resize(buf_.size() + recv_size);
}

void RequestBuilder::AdjustBufferSize_(size_t bytes_recvd) {
    parser_.remaining_length += bytes_recvd;
    LOG(DEBUG) << "Builder::AdjustBufferSize (" << bytes_recvd << ")";
    LOG(DEBUG) << "buf_size(): " << buf_.size() << "; end_idx_: " << parser_.end_idx << "; remaining_length: " << parser_.remaining_length;
    if (bytes_recvd >= 0 && (buf_.size() > (parser_.end_idx + parser_.remaining_length + bytes_recvd))) {
        LOG(DEBUG) << "buf_resize: " << buf_.size() << " -> " << parser_.end_idx + parser_.remaining_length + bytes_recvd;
        buf_.resize(parser_.end_idx + parser_.remaining_length + bytes_recvd);
    }
}

void RequestBuilder::Build(size_t bytes_recvd)
{
    LOG(DEBUG) << "RequestBuilder::Build(" << bytes_recvd << ")";
    AdjustBufferSize_(bytes_recvd);
    if (parser_.HasReachedEnd() && bytes_recvd == 0) {
        rq_.status = RQ_BAD;
        builder_status_ = RB_DONE;
        return ;
    }

    while (CanBuild_()) {
        LOG(DEBUG) << "Buildloop: State: " << build_state_;
        char c = ' ';
        if (IsProcessingState_(build_state_) && !parser_.HasReachedEnd()) {
            parser_.Advance(1);
            c = parser_.Peek();
            NullTerminatorCheck_(c);
        }
        BuildState old_state = build_state_;
        switch (build_state_) {
            case BS_METHOD:
                build_state_ = BuildMethod_();
                break;
            case BS_URI:
                build_state_ = BuildUri_(c);
                break;
            case BS_VERSION:
                build_state_ = BuildVersion_();
                break;
            case BS_BETWEEN_HEADERS:
                build_state_ = CheckForNextHeader_(c);
                break;
            case BS_HEADER_KEY:
                build_state_ = BuildHeaderKey_(c);
                break;
            case BS_HEADER_KEY_VAL_SEP:
                build_state_ = ParseHeaderKeyValSep_(c);
                break;
            case BS_HEADER_VALUE:
                build_state_ = BuildHeaderValue_(c);
                break;
            case BS_CHECK_FOR_BODY:
                build_state_ = CheckForBody_();
                if (IsBodyReadingState_(build_state_)) {
                    builder_status_ = http::RB_NEED_INFO_FROM_SERVER;
                    return;
                }
                break;
            case BS_CHECK_BODY_REGULAR_LENGTH:
                build_state_ = CheckBodyRegularLength_();
                break;
            case BS_BODY_REGULAR:
                build_state_ = BuildBodyRegular_();
                break;
            case BS_BODY_CHUNK_SIZE:
                build_state_ = BuildBodyChunkSize_(c);
                break;
            case BS_BODY_CHUNK_CONTENT:
                build_state_ = BuildBodyChunkContent_();
                break;
            case BS_END:
                break;
            case BS_BAD_REQUEST:
                break;
        }
        if (build_state_ != old_state) {
            parser_.UpdateBeginIdx();
        }
    }
    if (build_state_ == BS_BAD_REQUEST) {
        rq_.status = RQ_BAD;
    }
    if (build_state_ == BS_END) {
        rq_.status = RQ_GOOD;
    }
    if (rq_.status != RQ_INCOMPLETE) {
        builder_status_ = RB_DONE;
    }
}

void RequestBuilder::ApplyServerInfo(size_t max_body_size)
{
    body_builder_.max_body_size = max_body_size;
    if (parser_.HasReachedEnd()) {
        builder_status_ = http::RB_NEED_DATA_FROM_CLIENT;
    } else {
        builder_status_ = http::RB_BUILDING;
    }
}

RqBuilderStatus RequestBuilder::builder_status() const {
    return builder_status_;
}

const Request& RequestBuilder::rq() const
{
    return rq_;
}

std::vector<char>& RequestBuilder::buf()
{
    return buf_;
}

RequestBuilder::BuildState RequestBuilder::BuildMethod_()
{
    if (parser_.ElementLen() == 4 && CompareBuf_("GET ", 4) == 0) {
        rq_.method = HTTP_GET;
    }
    if (parser_.ElementLen() == 5 && CompareBuf_("POST ", 5) == 0) {
        rq_.method = HTTP_POST;
    }
    if (parser_.ElementLen() == 7 && CompareBuf_("DELETE ", 7) == 0) {
        rq_.method = HTTP_DELETE;
    }
    if (rq_.method != HTTP_NO_METHOD) {
        return BS_URI;
    }
    if (parser_.ElementLen() > 7) {
        return BS_BAD_REQUEST;
    }
    return BS_METHOD;
}

//https://datatracker.ietf.org/doc/html/rfc2616#page-17
// https://datatracker.ietf.org/doc/html/rfc2396
RequestBuilder::BuildState RequestBuilder::BuildUri_(char c)
{
    if (c == ' ') {
        if (parser_.ElementLen() >= 1) {
            rq_.uri = parser_.ExtractElementNoEOL();
            return BS_VERSION;
        } else {
            return BS_BAD_REQUEST;
        }
    }
    return BS_URI;
}

RequestBuilder::BuildState RequestBuilder::BuildVersion_(void)
{
    if (parser_.ElementLen() == 10 && CompareBuf_("HTTP/0.9\r\n", 10) == 0) {
        rq_.version = HTTP_0_9;
    }
    if (parser_.ElementLen() == 10 && CompareBuf_("HTTP/1.0\r\n", 10) == 0) {
        rq_.version = HTTP_1_0;
    }
    if (parser_.ElementLen() == 10 && CompareBuf_("HTTP/1.1\r\n", 10) == 0) {
        rq_.version = HTTP_1_1;
    }
    if (parser_.ElementLen() == 8 && CompareBuf_("HTTP/2\r\n", 8) == 0) {
        rq_.version = HTTP_2;
    }
    if (parser_.ElementLen() == 8 && CompareBuf_("HTTP/3\r\n", 8) == 0) {
        rq_.version = HTTP_3;
    }
    if (rq_.version != HTTP_NO_VERSION) {
        return BS_BETWEEN_HEADERS;
    }
    if (parser_.ElementLen() > 10) {
        return BS_BAD_REQUEST;
    }
    return BS_VERSION;
}

RequestBuilder::BuildState RequestBuilder::CheckForNextHeader_(char c)
{
    if (parser_.ElementLen() == 1 && c != EOL_CARRIAGE_RETURN) {
        parser_.Advance(-1);
        return BS_HEADER_KEY;
    }
    if (CheckForEOL_()) {
        return BS_CHECK_FOR_BODY;
    }
    return BS_BETWEEN_HEADERS;
}

RequestBuilder::BuildState RequestBuilder::BuildHeaderKey_(char c)
{
    if (c == ':') {
        if (parser_.ElementLen() == 1) {
            return BS_BAD_REQUEST;
        } else {
            // header_key_ = std::string(buf_.data() + parser_.element_begin_idx, parser_.ElementLen() - 1);
            header_key_ = parser_.ExtractElementNoEOL();
            // TODO: check if header_key already exists in map (no duplicates allowed)
            // also: check length of header keys/values and total number of headers
            return BS_HEADER_KEY_VAL_SEP;
        }
    } else if (!(std::isalnum(c) || (parser_.ElementLen() > 1 && c == '-'))) {
        return BS_BAD_REQUEST;
    }
    buf_[parser_.end_idx - 1] = std::tolower(c);
    return BS_HEADER_KEY;
}

RequestBuilder::BuildState RequestBuilder::ParseHeaderKeyValSep_(char c)
{
    if (std::isspace(c)) {
        return BS_HEADER_KEY_VAL_SEP;
    }
    if (parser_.ElementLen() < 2) {
        return BS_BAD_REQUEST;
    }
    parser_.Advance(-1);
    return BS_HEADER_VALUE;
}

// RFC: https://datatracker.ietf.org/doc/html/rfc2616#page-17
RequestBuilder::BuildState RequestBuilder::BuildHeaderValue_(char c)
{
    if (CheckForEOL_()) {
        rq_.headers[header_key_] = parser_.ExtractElementEOL();
        // rq_.headers[header_key_] = std::string(buf_.data() + parser_.element_begin_idx, parser_.ElementLen() - 2);
        return BS_BETWEEN_HEADERS;
    }
    if (parser_.ElementLen() > 1 && buf_[parser_.end_idx - 2] == EOL_CARRIAGE_RETURN) {
        return BS_BAD_REQUEST;
    }
    if (c != EOL_CARRIAGE_RETURN && !std::isprint(c)) { // TODO: additional checks for valid characters...
        return BS_BAD_REQUEST;
    }
    return BS_HEADER_VALUE;
}

RequestBuilder::BuildState RequestBuilder::CheckForBody_(void)
{
    if (rq_.method == HTTP_GET || rq_.method == HTTP_DELETE) {
        return BS_END;
    }
    std::pair<bool, std::string> content_length = rq_.GetHeaderVal("content-length");
    std::pair<bool, std::string> transfer_encoding = rq_.GetHeaderVal("transfer-encoding");
    if (content_length.first && transfer_encoding.first) {
        return BS_BAD_REQUEST;
    }
    if (transfer_encoding.second == "chunked") {
        body_builder_.chunked = true;
        return BS_BODY_CHUNK_SIZE;
    }
    if (content_length.first) {
        std::pair<bool, size_t> content_length_num = utils::StrToNumericNoThrow<size_t>(content_length.second);
        if (content_length_num.first) {
            body_builder_.remaining_length = content_length_num.second; // TODO: content-length limits?
            return BS_CHECK_BODY_REGULAR_LENGTH;
        } else {
            return BS_BAD_REQUEST;
        }
    }
    if (rq_.method == HTTP_POST) {
        return BS_BAD_REQUEST;
    }
    return BS_END;
}

RequestBuilder::BuildState RequestBuilder::CheckBodyRegularLength_(void) {
    if (body_builder_.remaining_length > body_builder_.max_body_size) {
        return BS_BAD_REQUEST;
    }
    body_builder_.body->resize(body_builder_.remaining_length);
    return BS_BODY_REGULAR;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyRegular_(void)
{
    if (parser_.HasReachedEnd()) {
        size_t copy_size = std::min(buf_.size() - parser_.element_begin_idx, body_builder_.remaining_length);
        std::memcpy(body_builder_.body->data() + body_builder_.body_idx, buf_.data() + parser_.element_begin_idx, copy_size);
        std::string str(buf_.data() + parser_.element_begin_idx, buf_.data() + parser_.element_begin_idx + copy_size);
        body_builder_.body_idx += copy_size;
        body_builder_.remaining_length -= copy_size;
        parser_.UpdateBeginIdx();
        if (body_builder_.remaining_length == 0) {
            return BS_END;
        }
    }
    return BS_BODY_REGULAR;
}

// https://datatracker.ietf.org/doc/html/rfc2616#section-3.5
RequestBuilder::BuildState RequestBuilder::BuildBodyChunkSize_(char c)
{
    buf_[parser_.end_idx - 1] = std::tolower(c);
    if (CheckForEOL_()) {
        // std::pair<bool, size_t> converted_size = utils::HexToNumericNoThrow<size_t>(
        //     std::string(buf_.data() + parser_.element_begin_idx, parser_.ElementLen() - 2));
         std::pair<bool, size_t> converted_size = utils::HexToNumericNoThrow<size_t>(
            parser_.ExtractElementEOL());
        if (!converted_size.first) {
            return BS_BAD_REQUEST;
        }
        body_builder_.remaining_length = converted_size.second; // TODO: check for chunk size limits
        if (rq_.body.size() + body_builder_.remaining_length > body_builder_.max_body_size) {
            return BS_BAD_REQUEST;
        }
        if (body_builder_.remaining_length == 0) {
            return BS_END;
        }
        return BS_BODY_CHUNK_CONTENT;
    }
    return BS_BODY_CHUNK_SIZE;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyChunkContent_(void)
{
    if (CheckForEOL_()) {
        if (parser_.ElementLen() - 2 != body_builder_.remaining_length) {
            return BS_BAD_REQUEST;
        }
        size_t old_sz = body_builder_.body->size();
        body_builder_.body->resize(old_sz + body_builder_.remaining_length);
        std::memcpy(body_builder_.body->data() + old_sz, buf_.data() + parser_.element_begin_idx, body_builder_.remaining_length);
        return BS_BODY_CHUNK_SIZE;
    }
    return BS_BODY_CHUNK_CONTENT;
}

bool RequestBuilder::CanBuild_(void) {
    if (build_state_ == BS_BAD_REQUEST || build_state_ == BS_END) {
        return false;
    }
    if (build_state_ == BS_BODY_REGULAR && parser_.ElementLen() > 0) {
        return true;
    }
    if (IsProcessingState_(build_state_) && parser_.HasReachedEnd()) {
        builder_status_ = http::RB_NEED_DATA_FROM_CLIENT;
        return false;
    }
    return true;
}

int RequestBuilder::CompareBuf_(const char* str, size_t len) const
{
    return std::strncmp(buf_.data() + parser_.element_begin_idx, str, len);
}


void RequestBuilder::NullTerminatorCheck_(char c)
{
    if (c == '\0' && build_state_ != BS_BODY_CHUNK_CONTENT) {
        build_state_ = BS_BAD_REQUEST;
    }
}

bool RequestBuilder::CheckForEOL_() const {
    if (parser_.ElementLen() < 2) {
        return false;
    }
    if (buf_[parser_.end_idx - 2] == EOL_CARRIAGE_RETURN && buf_[parser_.end_idx - 1] == EOL_LINE_FEED) {
        return true;
    }
    return false;
}

bool RequestBuilder::IsBodyReadingState_(BuildState state) const
{
    return (state == BS_BODY_CHUNK_SIZE || state == BS_CHECK_BODY_REGULAR_LENGTH);
}

bool RequestBuilder::IsProcessingState_(BuildState state) const
{
    return (state != BS_CHECK_FOR_BODY && state != BS_CHECK_BODY_REGULAR_LENGTH );
}

}  // namespace http
