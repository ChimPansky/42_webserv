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
    : builder_status_(RB_BUILDING), begin_idx_(0), end_idx_(0), build_state_(BS_METHOD), body_builder_(&rq_.body)
{}

RequestBuilder::BodyBuilder::BodyBuilder(std::vector<char> *rq_body)
    : body(rq_body), chunked(false), body_idx(0), remaining_length(0), max_body_size(0)
{}

void RequestBuilder::PrepareToRecvData(size_t recv_size) {
    buf_.resize(buf_.size() + recv_size);
}

void RequestBuilder::AdjustBufferSize_(size_t bytes_recvd) {
    if (bytes_recvd >= 0 && (buf_.size() > (end_idx_ + bytes_recvd))) {
        buf_.resize(end_idx_ + bytes_recvd);
    }
}

void RequestBuilder::Build(size_t bytes_recvd)
{
    AdjustBufferSize_(bytes_recvd);
    if (HasReachedEndOfBuffer_() && bytes_recvd == 0) {
        rq_.status = RQ_BAD;
        builder_status_ = RB_DONE;
        return ;
    }

    while (CanBuild_()) {
        char c = ' ';
        if (IsProcessingState_(build_state_) && !HasReachedEndOfBuffer_()) {
            c = buf_[end_idx_++];
            NullTerminatorCheck_(c);
        }
        bool state_changed = false;
        switch (build_state_) {
            case BS_METHOD:
                build_state_ = BuildMethod_();
                state_changed = (build_state_ != BS_METHOD ? true : false);
                break;
            case BS_URI:
                build_state_ = BuildUri_(c);
                state_changed = (build_state_ != BS_URI ? true : false);
                break;
            case BS_VERSION:
                build_state_ = BuildVersion_();
                state_changed = (build_state_ != BS_VERSION ? true : false);
                break;
            case BS_BETWEEN_HEADERS:
                build_state_ = CheckForNextHeader_(c);
                state_changed = (build_state_ != BS_BETWEEN_HEADERS ? true : false);
                break;
            case BS_HEADER_KEY:
                build_state_ = BuildHeaderKey_(c);
                state_changed = (build_state_ != BS_HEADER_KEY ? true : false);
                break;
            case BS_HEADER_KEY_VAL_SEP:
                build_state_ = ParseHeaderKeyValSep_(c);
                state_changed = (build_state_ != BS_HEADER_KEY_VAL_SEP ? true : false);
                break;
            case BS_HEADER_VALUE:
                build_state_ = BuildHeaderValue_(c);
                state_changed = (build_state_ != BS_HEADER_VALUE ? true : false);
                break;
            case BS_CHECK_FOR_BODY:
                build_state_ = CheckForBody_();
                state_changed = (build_state_ != BS_CHECK_FOR_BODY ? true : false);
                if (IsBodyReadingState_(build_state_)) {
                    builder_status_ = http::RB_NEED_INFO_FROM_SERVER;
                    return;
                }
                break;
            case BS_CHECK_BODY_REGULAR_LENGTH:
                build_state_ = CheckBodyRegularLength_();
                state_changed = (build_state_ != BS_CHECK_BODY_REGULAR_LENGTH ? true : false);
                break;
            case BS_BODY_REGULAR:
                build_state_ = BuildBodyRegular_();
                state_changed = (build_state_ != BS_BODY_REGULAR ? true : false);
                break;
            case BS_BODY_CHUNK_SIZE:
                build_state_ = BuildBodyChunkSize_(c);
                state_changed = (build_state_ != BS_BODY_CHUNK_SIZE ? true : false);
                break;
            case BS_BODY_CHUNK_CONTENT:
                build_state_ = BuildBodyChunkContent_();
                state_changed = (build_state_ != BS_BODY_CHUNK_CONTENT ? true : false);
                break;
            case BS_END:
                break;
            case BS_BAD_REQUEST:
                break;
        }
        if (state_changed) {
            UpdateBeginIdx_();
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
    if (HasReachedEndOfBuffer_()) {
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
    if (ParseLen_() == 4 && CompareBuf_("GET ", 4) == 0) {
        rq_.method = HTTP_GET;
    }
    if (ParseLen_() == 5 && CompareBuf_("POST ", 5) == 0) {
        rq_.method = HTTP_POST;
    }
    if (ParseLen_() == 7 && CompareBuf_("DELETE ", 7) == 0) {
        rq_.method = HTTP_DELETE;
    }
    if (rq_.method != HTTP_NO_METHOD) {
        return BS_URI;
    }
    if (ParseLen_() > 7) {
        return BS_BAD_REQUEST;
    }
    return BS_METHOD;
}

//https://datatracker.ietf.org/doc/html/rfc2616#page-17
// https://datatracker.ietf.org/doc/html/rfc2396
RequestBuilder::BuildState RequestBuilder::BuildUri_(char c)
{
    if (c == ' ') {
        if (ParseLen_() > 1) {
            rq_.uri = std::string(buf_.data() + begin_idx_, buf_.data() + end_idx_ - 1);
            return BS_VERSION;
        } else {
            return BS_BAD_REQUEST;
        }
    }
    return BS_URI;
}

RequestBuilder::BuildState RequestBuilder::BuildVersion_(void)
{
    if (ParseLen_() == 10 && CompareBuf_("HTTP/0.9\r\n", 10) == 0) {
        rq_.version = HTTP_0_9;
    }
    if (ParseLen_() == 10 && CompareBuf_("HTTP/1.0\r\n", 10) == 0) {
        rq_.version = HTTP_1_0;
    }
    if (ParseLen_() == 10 && CompareBuf_("HTTP/1.1\r\n", 10) == 0) {
        rq_.version = HTTP_1_1;
    }
    if (ParseLen_() == 8 && CompareBuf_("HTTP/2\r\n", 8) == 0) {
        rq_.version = HTTP_2;
    }
    if (ParseLen_() == 8 && CompareBuf_("HTTP/3\r\n", 8) == 0) {
        rq_.version = HTTP_3;
    }
    if (rq_.version != HTTP_NO_VERSION) {
        return BS_BETWEEN_HEADERS;
    }
    if (ParseLen_() > 10) {
        return BS_BAD_REQUEST;
    }
    return BS_VERSION;
}

RequestBuilder::BuildState RequestBuilder::CheckForNextHeader_(char c)
{
    if (ParseLen_() == 1 && c != EOL_CARRIAGE_RETURN) {
        end_idx_--;
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
        if (ParseLen_() == 1) {
            return BS_BAD_REQUEST;
        } else {
            header_key_ = std::string(buf_.data() + begin_idx_, ParseLen_() - 1);
            // TODO: check if header_key already exists in map (no duplicates allowed)
            // also: check length of header keys/values and total number of headers
            return BS_HEADER_KEY_VAL_SEP;
        }
    } else if (!(std::isalnum(c) || (ParseLen_() > 1 && c == '-'))) {
        return BS_BAD_REQUEST;
    }
    buf_[end_idx_ - 1] = std::tolower(c);
    return BS_HEADER_KEY;
}

RequestBuilder::BuildState RequestBuilder::ParseHeaderKeyValSep_(char c)
{
    if (std::isspace(c)) {
        return BS_HEADER_KEY_VAL_SEP;
    }
    if (ParseLen_() < 2) {
        return BS_BAD_REQUEST;
    }
    end_idx_--;
    return BS_HEADER_VALUE;
}

// RFC: https://datatracker.ietf.org/doc/html/rfc2616#page-17
RequestBuilder::BuildState RequestBuilder::BuildHeaderValue_(char c)
{
    if (CheckForEOL_()) {
        rq_.headers[header_key_] = std::string(buf_.data() + begin_idx_, ParseLen_() - 2);
        return BS_BETWEEN_HEADERS;
    }
    if (ParseLen_() > 1 && buf_[end_idx_ - 2] == EOL_CARRIAGE_RETURN) {
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
    if (HasReachedEndOfBuffer_()) {
        size_t copy_size = std::min(buf_.size() - begin_idx_, body_builder_.remaining_length);
        std::memcpy(body_builder_.body->data() + body_builder_.body_idx, buf_.data() + begin_idx_, copy_size);
        std::string str(buf_.data() + begin_idx_, buf_.data() + begin_idx_ + copy_size);
        body_builder_.body_idx += copy_size;
        body_builder_.remaining_length -= copy_size;
        UpdateBeginIdx_();
        if (body_builder_.remaining_length == 0) {
            return BS_END;
        }
    }
    return BS_BODY_REGULAR;
}

// https://datatracker.ietf.org/doc/html/rfc2616#section-3.5
RequestBuilder::BuildState RequestBuilder::BuildBodyChunkSize_(char c)
{
    buf_[end_idx_ - 1] = std::tolower(c);
    if (CheckForEOL_()) {
        std::pair<bool, size_t> converted_size = utils::HexToNumericNoThrow<size_t>(
            std::string(buf_.data() + begin_idx_, ParseLen_() - 2));
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
        if (ParseLen_() - 2 != body_builder_.remaining_length) {
            return BS_BAD_REQUEST;
        }
        size_t old_sz = body_builder_.body->size();
        body_builder_.body->resize(old_sz + body_builder_.remaining_length);
        std::memcpy(body_builder_.body->data() + old_sz, buf_.data() + begin_idx_, body_builder_.remaining_length);
        return BS_BODY_CHUNK_SIZE;
    }
    return BS_BODY_CHUNK_CONTENT;
}

bool RequestBuilder::CanBuild_(void) {
    if (build_state_ == BS_BAD_REQUEST || build_state_ == BS_END) {
        return false;
    }
    if (build_state_ == BS_BODY_REGULAR && ParseLen_() > 0) {
        return true;
    }
    if (IsProcessingState_(build_state_) && HasReachedEndOfBuffer_()) {
        builder_status_ = http::RB_NEED_DATA_FROM_CLIENT;
        return false;
    }
    return true;
}

size_t RequestBuilder::ParseLen_() const
{
    return end_idx_ - begin_idx_;
}

int RequestBuilder::CompareBuf_(const char* str, size_t len) const
{
    return std::strncmp(buf_.data() + begin_idx_, str, len);
}


void RequestBuilder::NullTerminatorCheck_(char c)
{
    if (c == '\0' && build_state_ != BS_BODY_CHUNK_CONTENT) {
        build_state_ = BS_BAD_REQUEST;
    }
}
void RequestBuilder::UpdateBeginIdx_()
{
    begin_idx_ = end_idx_;
}

bool RequestBuilder::CheckForEOL_() const {
    if (ParseLen_() < 2) {
        return false;
    }
    if (buf_[end_idx_ - 2] == EOL_CARRIAGE_RETURN && buf_[end_idx_ - 1] == EOL_LINE_FEED) {
        return true;
    }
    return false;
}

bool RequestBuilder::HasReachedEndOfBuffer_(void) const
{
    return end_idx_ == buf_.size();
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
