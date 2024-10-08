#include "RequestBuilder.h"

#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#include "Request.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "utils.h"

namespace http {

RequestBuilder::RequestBuilder()
    : crlf_counter_(0), begin_idx_(0), end_idx_(0), build_state_(BS_METHOD), found_space_(false), needs_info_from_server_(false), body_builder_(&rq_.body)
{}

RequestBuilder::BodyBuilder::BodyBuilder(std::vector<char> *rq_body)
    : body(rq_body), chunked(false), chunk_size(0), idx(0), remaining_length(0), max_body_size(0)
{}

bool RequestBuilder::BodyBuilder::Complete() const
{
    return remaining_length == 0;
}

const Request& RequestBuilder::rq() const
{
    return rq_;
}

bool RequestBuilder::needs_info_from_server() const
{
    return needs_info_from_server_;
}

void RequestBuilder::set_max_body_size(size_t max_body_size)
{
    body_builder_.max_body_size = max_body_size;
}

std::vector<char>& RequestBuilder::buf()
{
    return buf_;
}

bool RequestBuilder::HasReachedEndOfBuffer() const
{
    return end_idx_ == buf_.size();
}

size_t RequestBuilder::ProcessBuffer(size_t bytes_read)
{
    size_t chars_processed = 0;
    if (bytes_read == 0 && ReadingBody_() && !body_builder_.Complete() && !needs_info_from_server_) {
        build_state_ = BS_BAD_REQUEST;
    }
    while (!IsReadyForResponse() && (!HasReachedEndOfBuffer() || build_state_ == BS_CHECK_FOR_BODY) && build_state_ != BS_BAD_REQUEST) {
        chars_processed++;
        char c = GetNextChar_();
        bool state_changed = false;
        NullTerminatorCheck_(c);
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
                if (ReadingBody_()) {
                    needs_info_from_server_ = true;
                    return chars_processed;
                }
                break;
            case BS_BODY_REGULAR_CHECK_LENGTH:
                build_state_ = CheckBodyRegularLength_();
                state_changed = (build_state_ != BS_BODY_REGULAR_CHECK_LENGTH ? true : false);
                break;
            case BS_BODY_REGULAR:
                needs_info_from_server_ = false;
                build_state_ = BuildBodyRegular_();
                state_changed = (build_state_ != BS_BODY_REGULAR ? true : false);
                break;
            case BS_BODY_CHUNK_SIZE:
                needs_info_from_server_ = false;
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
    if (build_state_ == BS_BAD_REQUEST || chars_processed == 0) {
        rq_.status = RQ_BAD;
    }
    if (build_state_ == BS_END) {
        rq_.status = RQ_GOOD;
    }
    return chars_processed;
}

bool RequestBuilder::IsReadyForResponse()
{
    // return (rq_.rq_complete || build_state_ == BS_BAD_REQUEST /*body_complete()*/);
    return (rq_.status != RQ_INCOMPLETE);
}
size_t RequestBuilder::ParseLen_() const
{
    return end_idx_ - begin_idx_;
}

char RequestBuilder::GetNextChar_()
{
    if (build_state_ == BS_CHECK_FOR_BODY || build_state_ == BS_BODY_REGULAR_CHECK_LENGTH) {
        return ' ';
    }
    return buf_[end_idx_++];
}

void RequestBuilder::NullTerminatorCheck_(char c)
{
    if (c == '\0' && build_state_ != BS_CHECK_FOR_BODY && build_state_ != BS_BODY_CHUNK_CONTENT) {
        build_state_ = BS_BAD_REQUEST;
    }
}

int RequestBuilder::CompareBuf_(const char* str, size_t len) const
{
    return std::strncmp(buf_.data() + begin_idx_, str, len);
}

void RequestBuilder::UpdateBeginIdx_()
{
    ////LOG(DEBUG) << "RequestBuilder::UpdateBeginIdx_";
    begin_idx_ = end_idx_;
}

bool RequestBuilder::ReadingBody_() const
{
    return (build_state_ == BS_BODY_REGULAR || build_state_ == BS_BODY_CHUNK_SIZE || build_state_ == BS_BODY_CHUNK_CONTENT);
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
    if (ParseLen_() == 1) {
        if (c == '\r') {
            crlf_counter_++;
            return BS_BETWEEN_HEADERS;
        }
        end_idx_--;
        return BS_HEADER_KEY;
    }
    if (ParseLen_() == 2) {
        if (c == '\n') {
            crlf_counter_ = 0;
            return BS_CHECK_FOR_BODY;
        }
        crlf_counter_ = 0;
        return BS_BAD_REQUEST;
    }
    return BS_BAD_REQUEST; // should never reach this point...
}

RequestBuilder::BuildState RequestBuilder::BuildHeaderKey_(char c)
{
    if (c == ':') {
        if (ParseLen_() == 1) {
            LOG(ERROR) << "Request-Header key is invalid: " << std::string(buf_.data() + begin_idx_, ParseLen_() - 1);
            return BS_BAD_REQUEST;
        } else {
            header_key_ = std::string(buf_.data() + begin_idx_, ParseLen_() - 1);
            found_space_ = false;
            // TODO: check if header_key already exists in map (no duplicates allowed)
            return BS_HEADER_KEY_VAL_SEP;
        }
    } else if (!(std::isalnum(c) || (ParseLen_() > 1 && c == '-'))) { // TODO: check this condition...
        return BS_BAD_REQUEST;
    }
    buf_[end_idx_ - 1] = std::tolower(c);
    return BS_HEADER_KEY;
}

RequestBuilder::BuildState RequestBuilder::ParseHeaderKeyValSep_(char c)
{
    if (std::isspace(c)) {
        found_space_ = true;
        return BS_HEADER_KEY_VAL_SEP;
    }
    if (!found_space_) {
        return BS_BAD_REQUEST;
    }
    end_idx_--;
    return BS_HEADER_VALUE;
}

// RFC: https://datatracker.ietf.org/doc/html/rfc2616#page-17
RequestBuilder::BuildState RequestBuilder::BuildHeaderValue_(char c)
{
    if (crlf_counter_ == 0) {
        if (c == '\r') {
            if (ParseLen_() <= 1) {
                return BS_BAD_REQUEST;
            }
            crlf_counter_++;
            return BS_HEADER_VALUE;
        }
        if (!std::isprint(c)) { // TODO: additional checks for valid characters...
            return BS_BAD_REQUEST;
        }
    }
    if (crlf_counter_ == 1) {
        if (c == '\n') {
            crlf_counter_ = 0;
            rq_.headers[header_key_] = std::string(buf_.data() + begin_idx_, ParseLen_() - 2);
            return BS_BETWEEN_HEADERS;
        }
        crlf_counter_ = 0;
        return BS_BAD_REQUEST;

    }
    return BS_HEADER_VALUE;
}

RequestBuilder::BuildState RequestBuilder::CheckForBody_(void)
{
    if (rq_.method == HTTP_GET || rq_.method == HTTP_DELETE) {
        return BS_END;
    }
    std::string content_length = rq_.GetHeaderVal("content-length");
    std::string transfer_encoding = rq_.GetHeaderVal("transfer-encoding");
    if (!content_length.empty() && !transfer_encoding.empty()) {
        return BS_BAD_REQUEST;
    }
    if (transfer_encoding == "chunked") {
        body_builder_.chunked = true;
        return BS_BODY_CHUNK_SIZE;
    }
    if (!content_length.empty()) {
        std::pair<bool, size_t> content_length_num = utils::StrToNumericNoThrow<size_t>(content_length);
        if (content_length_num.first) {
            body_builder_.remaining_length = content_length_num.second; // TODO: content-length limits?
            body_builder_.body->resize(content_length_num.second);
        } else {
            return BS_BAD_REQUEST;
        }
        if (body_builder_.remaining_length != 0) {
            return BS_BODY_REGULAR;
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
    return BS_BODY_REGULAR;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyRegular_(void)
{
    if (HasReachedEndOfBuffer()) {
        size_t copy_size = std::min(buf_.size() - begin_idx_, body_builder_.remaining_length);
        std::memcpy(body_builder_.body->data() + body_builder_.idx, buf_.data() + begin_idx_, copy_size);
        body_builder_.idx += copy_size;
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
    if (crlf_counter_ == 0) {
        if (c == '\r') {
            crlf_counter_++;
        }
        return BS_BODY_CHUNK_SIZE;
    }
    if (crlf_counter_ == 1) {
        if (c == '\n') {
            crlf_counter_ = 0;
            std::pair<bool, size_t> converted_size = HexStrToSizeT(std::string(buf_.data() + begin_idx_, ParseLen_() - 2));
            if (!converted_size.first) {
                return BS_BAD_REQUEST;
            }
            body_builder_.chunk_size = converted_size.second; // TODO: check for chunk size limits
            if (rq_.body.size() + body_builder_.chunk_size > body_builder_.max_body_size) {
                return BS_BAD_REQUEST;
            }
            if (body_builder_.chunk_size == 0) {
                return BS_END;
            }
            return BS_BODY_CHUNK_CONTENT;
        }
        crlf_counter_ = 0;
        return BS_BAD_REQUEST;
    }
    return BS_BODY_CHUNK_SIZE;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyChunkContent_(void)
{
    if (ParseLen_() == body_builder_.chunk_size) {
        size_t old_sz = body_builder_.body->size();
        body_builder_.body->resize(old_sz + body_builder_.chunk_size);
        std::memcpy(body_builder_.body->data() + old_sz, buf_.data() + begin_idx_, body_builder_.chunk_size);
        return BS_BODY_CHUNK_CONTENT;
    }
    if (ParseLen_() == body_builder_.chunk_size + 1) {
        if (buf_[end_idx_ - 1] != '\r') {
            return BS_BAD_REQUEST;
        }
        return BS_BODY_CHUNK_CONTENT;
    }
    if (ParseLen_() == body_builder_.chunk_size + 2) {
        if (buf_[end_idx_ - 1] != '\n') {
            return BS_BAD_REQUEST;
        }
        return BS_BODY_CHUNK_SIZE;
    }
    return BS_BODY_CHUNK_CONTENT;
}

}  // namespace http
