#include "RequestBuilder.h"
#include "Request.h"

#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#include <logger.h>
#include <utils.h>

namespace http {

RequestBuilder::RequestBuilder()
    : builder_status_(RB_BUILDING), build_state_(BS_METHOD), body_builder_(&rq_.body)
{}

RequestBuilder::BodyBuilder::BodyBuilder(std::vector<char>* rq_body)
    : body(rq_body), chunked(false), body_idx(0), remaining_length(0), max_body_size(0)
{}

void RequestBuilder::PrepareToRecvData(size_t recv_size)
{
    parser_.PrepareToRecvData(recv_size);
}

void RequestBuilder::AdjustBufferSize(size_t bytes_recvd)
{
    parser_.AdjustBufferSize(bytes_recvd);
}

void RequestBuilder::Build(size_t bytes_recvd)
{
    if (parser_.EndOfBuffer() && bytes_recvd == 0) {
        rq_.status = RQ_BAD;
        builder_status_ = RB_DONE;
        return;
    }
    while (CanBuild_()) {
        if (IsParsingState_(build_state_)) {       // todo: check for \0 within states...
            NullTerminatorCheck_(parser_.Peek());  // can there be \0 in body???
        }
        BuildState old_state = build_state_;
        switch (build_state_) {
            case BS_METHOD:             build_state_ = BuildMethod_(); break;
            case BS_URI:                build_state_ = BuildUri_(); break;
            case BS_VERSION:            build_state_ = BuildVersion_(); break;
            case BS_BETWEEN_HEADERS:    build_state_ = CheckForNextHeader_(); break;
            case BS_HEADER_KEY:         build_state_ = BuildHeaderKey_(); break;
            case BS_HEADER_KEY_VAL_SEP: build_state_ = ParseHeaderKeyValSep_(); break;
            case BS_HEADER_VALUE:       build_state_ = BuildHeaderValue_(); break;
            case BS_CHECK_FOR_BODY: {
                build_state_ = CheckForBody_();
                if (build_state_ == BS_BODY_CHUNK_SIZE ||
                    build_state_ == BS_CHECK_BODY_REGULAR_LENGTH) {
                }
                builder_status_ = http::RB_NEED_INFO_FROM_SERVER;
                break;
            }
            case BS_CHECK_BODY_REGULAR_LENGTH:  build_state_ = CheckBodyRegularLength_(); break;
            case BS_BODY_REGULAR:               build_state_ = BuildBodyRegular_(); break;
            case BS_BODY_CHUNK_SIZE:            build_state_ = BuildBodyChunkSize_(); break;
            case BS_BODY_CHUNK_CONTENT:         build_state_ = BuildBodyChunkContent_(); break;
            case BS_BAD_REQUEST:
            case BS_END: {}
        }
        if (build_state_ != old_state) {
            parser_.StartNewElement();
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
    if (parser_.EndOfBuffer()) {
        builder_status_ = http::RB_NEED_DATA_FROM_CLIENT;
    } else {
        builder_status_ = http::RB_BUILDING;
    }
}

RqBuilderStatus RequestBuilder::builder_status() const
{
    return builder_status_;
}

const Request& RequestBuilder::rq() const
{
    return rq_;
}

std::vector<char>& RequestBuilder::buf()
{
    return parser_.buf();
}

RequestBuilder::BuildState RequestBuilder::BuildMethod_()
{
    while (!parser_.EndOfBuffer()) {
        if (parser_.ElementLen() == 4 && parser_.CompareBuf_("GET ", 4) == 0) {
            rq_.method = HTTP_GET;
        }
        if (parser_.ElementLen() == 5 && parser_.CompareBuf_("POST ", 5) == 0) {
            rq_.method = HTTP_POST;
        }
        if (parser_.ElementLen() == 7 && parser_.CompareBuf_("DELETE ", 7) == 0) {
            rq_.method = HTTP_DELETE;
        }
        if (rq_.method != HTTP_NO_METHOD) {
            parser_.Advance();
            return BS_URI;
        }
        if (parser_.ElementLen() > 7) {
            return BS_BAD_REQUEST;
        }
        parser_.Advance(1);
    }
    return BS_METHOD;
}

// https://datatracker.ietf.org/doc/html/rfc2616#page-17
//  https://datatracker.ietf.org/doc/html/rfc2396
RequestBuilder::BuildState RequestBuilder::BuildUri_()
{
    while (!parser_.EndOfBuffer()) {
        if (parser_.ExceededLineLimit() || parser_.ElementLen() > RQ_URI_LEN_LIMIT) {
            return BS_BAD_REQUEST;  // todo: 414 Request-URI Too Long
        }
        if (parser_.Peek() == ' ') {
            if (parser_.ElementLen() > 1) {
                rq_.uri = parser_.ExtractElement();
                parser_.Advance();
                return BS_VERSION;
            } else {
                return BS_BAD_REQUEST;
            }
        }
        parser_.Advance();
    }
    return BS_URI;
}

RequestBuilder::BuildState RequestBuilder::BuildVersion_()
{
    while (!parser_.EndOfBuffer()) {
        if (parser_.ElementLen() == 10 && parser_.CompareBuf_("HTTP/0.9\r\n", 10) == 0) {
            rq_.version = HTTP_0_9;
        }
        if (parser_.ElementLen() == 10 && parser_.CompareBuf_("HTTP/1.0\r\n", 10) == 0) {
            rq_.version = HTTP_1_0;
        }
        if (parser_.ElementLen() == 10 && parser_.CompareBuf_("HTTP/1.1\r\n", 10) == 0) {
            rq_.version = HTTP_1_1;
        }
        if (parser_.ElementLen() == 8 && parser_.CompareBuf_("HTTP/2\r\n", 8) == 0) {
            rq_.version = HTTP_2;
        }
        if (parser_.ElementLen() == 8 && parser_.CompareBuf_("HTTP/3\r\n", 8) == 0) {
            rq_.version = HTTP_3;
        }
        if (rq_.version != HTTP_NO_VERSION) {
            parser_.Advance();
            return BS_BETWEEN_HEADERS;
        }
        if (parser_.ElementLen() > 10) {
            return BS_BAD_REQUEST;
        }
        parser_.Advance();
    }
    return BS_VERSION;
}

RequestBuilder::BuildState RequestBuilder::CheckForNextHeader_()
{
    while (!parser_.EndOfBuffer()) {
        if (parser_.ElementLen() == 1 && parser_.Peek() != EOL_CARRIAGE_RETURN) {
            return BS_HEADER_KEY;
        }
        if (CheckForEOL_()) {
            parser_.Advance();
            return BS_CHECK_FOR_BODY;
        }
        parser_.Advance();
    }
    if (CheckForEOL_()) {
        return BS_CHECK_FOR_BODY;
    }
    return BS_BETWEEN_HEADERS;
}

RequestBuilder::BuildState RequestBuilder::BuildHeaderKey_()
{
    while (!parser_.EndOfBuffer()) {
        char c = parser_.Peek();
        if (c == ':') {
            if (parser_.ElementLen() == 1) {
                return BS_BAD_REQUEST;
            } else {
                header_key_ = parser_.ExtractElement();
                parser_.Advance();
                // TODO: check if header_key already exists in map (no duplicates allowed)
                // also: check length of header keys/values and total number of headers
                return BS_HEADER_KEY_VAL_SEP;
            }
        } else if (!(std::isalnum(c) || (parser_.ElementLen() > 1 && c == '-'))) {
            return BS_BAD_REQUEST;
        }
        parser_[parser_.element_end_idx()] = std::tolower(parser_[parser_.element_end_idx()]);
        parser_.Advance();
    }
    return BS_HEADER_KEY;
}

RequestBuilder::BuildState RequestBuilder::ParseHeaderKeyValSep_()
{
    while (!parser_.EndOfBuffer()) {
        if (std::isspace(parser_.Peek())) {
            parser_.Advance();
            continue;
        }
        if (parser_.ElementLen() < 2) {
            return BS_BAD_REQUEST;
        }
        return BS_HEADER_VALUE;
    }
    return BS_HEADER_KEY_VAL_SEP;
}

// RFC: https://datatracker.ietf.org/doc/html/rfc2616#page-17
RequestBuilder::BuildState RequestBuilder::BuildHeaderValue_()
{
    while (!parser_.EndOfBuffer()) {
        char c = parser_.Peek();
        if (CheckForEOL_()) {
            rq_.headers[header_key_] = parser_.ExtractElement(-1);
            parser_.Advance();
            return BS_BETWEEN_HEADERS;
        }
        if (parser_.ElementLen() > 1 && parser_.Peek(-1) == EOL_CARRIAGE_RETURN) {
            return BS_BAD_REQUEST;
        }
        if (c != EOL_CARRIAGE_RETURN &&
            !std::isprint(c)) {  // TODO: additional checks for valid characters...
            return BS_BAD_REQUEST;
        }
        parser_.Advance();
    }
    return BS_HEADER_VALUE;
}

RequestBuilder::BuildState RequestBuilder::CheckForBody_()
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
        std::pair<bool, size_t> content_length_num =
            utils::StrToNumericNoThrow<size_t>(content_length.second);
        if (content_length_num.first) {
            body_builder_.remaining_length =
                content_length_num.second;  // TODO: content-length limits?
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

RequestBuilder::BuildState RequestBuilder::CheckBodyRegularLength_()
{
    if (body_builder_.remaining_length > body_builder_.max_body_size) {
        return BS_BAD_REQUEST;
    }
    body_builder_.body->resize(body_builder_.remaining_length);
    return BS_BODY_REGULAR;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyRegular_()
{
    while (!parser_.EndOfBuffer() && body_builder_.remaining_length > 0) {
        parser_.Advance();
    }
    size_t copy_size = std::min(parser_.ElementLen(), body_builder_.remaining_length);
    std::memcpy(body_builder_.body->data() + body_builder_.body_idx,
                parser_.buf().data() + parser_.element_begin_idx(), copy_size);
    body_builder_.body_idx += copy_size;
    body_builder_.remaining_length -= copy_size;
    parser_.StartNewElement();
    if (body_builder_.remaining_length == 0) {
        return BS_END;
    }
    parser_.Advance();
    return BS_BODY_REGULAR;
}

// https://datatracker.ietf.org/doc/html/rfc2616#section-3.5
RequestBuilder::BuildState RequestBuilder::BuildBodyChunkSize_()
{
    while (true) {
        parser_[parser_.element_end_idx() - 1] =
            std::tolower(parser_[parser_.element_end_idx() - 1]);
        if (CheckForEOL_()) {
            std::pair<bool, size_t> converted_size =
                utils::HexToUnsignedNumericNoThrow<size_t>(parser_.ExtractElement(-1));
            if (!converted_size.first) {
                return BS_BAD_REQUEST;
            }
            body_builder_.remaining_length =
                converted_size.second;  // TODO: check for chunk size limits
            if (rq_.body.size() + body_builder_.remaining_length > body_builder_.max_body_size) {
                return BS_BAD_REQUEST;
            }
            if (body_builder_.remaining_length == 0) {
                return BS_END;
            }
            parser_.Advance();
            return BS_BODY_CHUNK_CONTENT;
        }
        if (parser_.EndOfBuffer()) {
            break;
        }
        // TODO: check for single \r...
        parser_.Advance();
    }
    return BS_BODY_CHUNK_SIZE;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyChunkContent_()
{
    while (!parser_.EndOfBuffer() && parser_.ElementLen() <= body_builder_.remaining_length + 2) {
        if (CheckForEOL_()) {
            if (parser_.ElementLen() - 2 != body_builder_.remaining_length) {
                return BS_BAD_REQUEST;
            }
            size_t old_sz = body_builder_.body->size();
            body_builder_.body->resize(old_sz + body_builder_.remaining_length);
            std::memcpy(body_builder_.body->data() + old_sz,
                        parser_.buf().data() + parser_.element_begin_idx(),
                        body_builder_.remaining_length);
            parser_.Advance();
            return BS_BODY_CHUNK_SIZE;
        }
        parser_.Advance();
    }
    return BS_BODY_CHUNK_CONTENT;
}

bool RequestBuilder::CanBuild_()
{
    if (build_state_ == BS_BAD_REQUEST || build_state_ == BS_END) {
        return false;
    }
    if (build_state_ == BS_BODY_REGULAR && parser_.ElementLen() > 0) {
        return true;
    }
    if (IsParsingState_(build_state_) && parser_.EndOfBuffer()) {
        builder_status_ = http::RB_NEED_DATA_FROM_CLIENT;
        return false;
    }
    return true;
}

void RequestBuilder::NullTerminatorCheck_(char c)
{
    if (c == '\0' && build_state_ != BS_BODY_CHUNK_CONTENT) {
        build_state_ = BS_BAD_REQUEST;
    }
}

bool RequestBuilder::CheckForEOL_() const
{
    if (parser_.ElementLen() < 2) {
        return false;
    }
    if (parser_.Peek(-1) == EOL_CARRIAGE_RETURN && parser_.Peek() == EOL_LINE_FEED) {
        return true;
    }
    return false;
}

bool RequestBuilder::IsParsingState_(BuildState state) const
{
    return (state != BS_CHECK_FOR_BODY && state != BS_CHECK_BODY_REGULAR_LENGTH);
}

}  // namespace http
