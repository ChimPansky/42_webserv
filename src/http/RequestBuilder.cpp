#include "RequestBuilder.h"

#include <cctype>
#include <cstring>
#include <iostream>
#include <vector>

#include "utils/logger.h"
#include "utils/utils.h"

namespace http {



RequestBuilder::RequestBuilder()
    : chunk_counter_(0), crlf_counter_(0), begin_idx_(0), end_idx_(0), parse_state_(PS_METHOD)
{}
const Request& RequestBuilder::rq() const
{
    return rq_;
}

std::vector<char>& RequestBuilder::buf()
{
    return buf_;
}

void RequestBuilder::ParseNext(void)
{
    ++chunk_counter_;
    //LOG(DEBUG) << "Parsing chunk no " << chunk_counter_;
    //LOG(DEBUG) << "buf_.size(): " << buf_.size() << "; end_idx_: " << end_idx_;
    LOG(DEBUG) << "buf_.size(): " << buf_.size() << "; end_idx_: " << end_idx_;
    if (buf_.size() == end_idx_) {  // 0 bytes received...
        LOG(DEBUG) << "buf.size() == end_idx -> bad_request";
        parse_state_ = PS_BAD_REQUEST;
    }
    while (end_idx_ < buf_.size() && !IsReadyForResponse()) {
        char c = buf_[end_idx_++];
        bool state_changed = false;
        NullTerminatorCheck_(c);
        switch (parse_state_) {
            case PS_METHOD:
                parse_state_ = ParseMethod_(c);
                state_changed = (parse_state_ != PS_METHOD ? true : false);
                break;
            case PS_URI:
                parse_state_ = ParseUri_(c);
                state_changed = (parse_state_ != PS_URI ? true : false);
                break;
            case PS_VERSION:
                parse_state_ = ParseVersion_(c);
                state_changed = (parse_state_ != PS_VERSION ? true : false);
                break;
            case PS_BETWEEN_HEADERS:
                parse_state_ = CheckForNextHeader_(c);
                state_changed = (parse_state_ != PS_BETWEEN_HEADERS ? true : false);
                break;
            case PS_HEADER_KEY:
                parse_state_ = ParseHeaderKey_(c);
                state_changed = (parse_state_ != PS_HEADER_KEY ? true : false);
                break;
            case PS_HEADER_KEY_VAL_SEP:
                parse_state_ = ParseHeaderKeyValSep_(c);
                state_changed = (parse_state_ != PS_HEADER_KEY_VAL_SEP ? true : false);
                break;
            case PS_HEADER_VALUE:
                parse_state_ = ParseHeaderValue_(c);
                state_changed = (parse_state_ != PS_HEADER_VALUE ? true : false);
                break;
            case PS_AFTER_HEADERS:
                parse_state_ = CheckForBody_();
                state_changed = (parse_state_ != PS_AFTER_HEADERS ? true : false);
                break;
            case PS_BODY:
                parse_state_ = ParseBody_(c);
                state_changed = (parse_state_ != PS_BODY ? true : false);
                break;
            case PS_END:
                break;
            case PS_BAD_REQUEST:
                break;
        }
        if (state_changed) {
            UpdateBeginIdx_();
        }
    }
    if (parse_state_ == PS_BAD_REQUEST) {
        LOG(DEBUG) << "After loop: PS_BAD_REQUEST -> bad_request";
        rq_.bad_request = true;
    }
}

bool RequestBuilder::IsReadyForResponse()
{
    //LOG(DEBUG) << "RequestBuilder::IsReadyForResponse: EOF: " << eof_checker_.end_of_file_
    //           << "Bad Request: " << rq_.bad_request_;
    return (rq_.rq_complete || parse_state_ == PS_BAD_REQUEST /*body_complete()*/);
}
size_t RequestBuilder::ParseLen_() const
{
    return end_idx_ - begin_idx_;
}

void RequestBuilder::NullTerminatorCheck_(char c)
{
    if (c == '\0' && parse_state_ != PS_BODY && !rq_.body.chunked) {
        LOG(DEBUG) << "Null char found -> bad_request";
        parse_state_ = PS_BAD_REQUEST;
    }
}

int RequestBuilder::CompareBuf_(const char* str, size_t len) const
{
    return std::strncmp(buf_.data() + begin_idx_, str, len);
}

void RequestBuilder::UpdateBeginIdx_()
{
    //LOG(DEBUG) << "RequestBuilder::UpdateBeginIdx_";
    begin_idx_ = end_idx_;
}

RequestBuilder::ParseState RequestBuilder::ParseMethod_(char c)
{
    (void)c;
    //LOG(DEBUG) << "Parsing Method... char: " << c;
    if (ParseLen_() == 4 && CompareBuf_("GET ", 4) == 0) {
        rq_.method = HTTP_GET;
        return PS_URI;
    }
    if (ParseLen_() == 5 && CompareBuf_("POST ", 5) == 0) {
        rq_.method = HTTP_POST;
        return PS_URI;
    }
    if (ParseLen_() == 7 && CompareBuf_("DELETE ", 7) == 0) {
        rq_.method = HTTP_DELETE;
        return PS_URI;
    }
    if (ParseLen_() > 7) {
        rq_.bad_request = true;
        return PS_BAD_REQUEST;
    }
    return PS_METHOD;
}

//https://datatracker.ietf.org/doc/html/rfc2616#page-17
// https://datatracker.ietf.org/doc/html/rfc2396
RequestBuilder::ParseState RequestBuilder::ParseUri_(char c)
{
    //LOG(DEBUG) << "Parsing URI... char: " << c;

    if (c == ' ') {
        if (ParseLen_() > 1) {
            rq_.uri = std::string(buf_.data() + begin_idx_, buf_.data() + end_idx_ - 1);
            return PS_VERSION;
        } else {
            return PS_BAD_REQUEST;
        }
    }
    return PS_URI;
}

RequestBuilder::ParseState RequestBuilder::ParseVersion_(char c)
{
    (void)c;
    LOG(DEBUG) << "Parsing Version... char: " << c;
    if (ParseLen_() == 10 && CompareBuf_("HTTP/0.9\r\n", 10) == 0) {
        rq_.version = HTTP_0_9;
        return PS_BETWEEN_HEADERS;
    }
    if (ParseLen_() == 10 && CompareBuf_("HTTP/1.0\r\n", 10) == 0) {
        rq_.version = HTTP_1_0;
        return PS_BETWEEN_HEADERS;
    }
    if (ParseLen_() == 10 && CompareBuf_("HTTP/1.1\r\n", 10) == 0) {
        rq_.version = HTTP_1_1;
        return PS_BETWEEN_HEADERS;
    }
    if (ParseLen_() == 8 && CompareBuf_("HTTP/2\r\n", 8) == 0) {
        rq_.version = HTTP_2;
        return PS_BETWEEN_HEADERS;
    }
    if (ParseLen_() == 8 && CompareBuf_("HTTP/3\r\n", 8) == 0) {
        rq_.version = HTTP_3;
        return PS_BETWEEN_HEADERS;
    }
    if (ParseLen_() > 10) {
        return PS_BAD_REQUEST;
    }
    return PS_VERSION;
}

RequestBuilder::ParseState RequestBuilder::CheckForNextHeader_(char c)
{
    LOG(DEBUG) << "Checking for next header... chr: " << c << " (" << (int)c << ")";
    if (ParseLen_() == 1) {
        if (c == '\r') {
            LOG(DEBUG) << "Found carriage return..";
            crlf_counter_++;
            return PS_BETWEEN_HEADERS;
        }
        LOG(DEBUG) << "Found random char -> end_idx_-- and go to PS_HEADER_KEY...";
        end_idx_--;
        return PS_HEADER_KEY;
    }
    if (ParseLen_() == 2) {
        if (c == '\n') {
            LOG(DEBUG) << "Found empty line -> End of Headers";
            crlf_counter_ = 0;
            return PS_AFTER_HEADERS;
        }
        LOG(DEBUG) << "Found random char after carriage return -> bad_request...";
        return PS_BAD_REQUEST;
    }
    return PS_BAD_REQUEST; // should never reach this point...
}

RequestBuilder::ParseState RequestBuilder::ParseHeaderKey_(char c)
{
    LOG(DEBUG) << "Parsing Header-Key... char: " << c;
    if (c == ':') {
        if (ParseLen_() == 1) {
            LOG(ERROR) << "Request-Header key is invalid: " << std::string(buf_.data() + begin_idx_, ParseLen_() - 1);
            return PS_BAD_REQUEST;
        } else {
            header_key_ = std::string(buf_.data() + begin_idx_, ParseLen_() - 1);
            return PS_HEADER_KEY_VAL_SEP;
        }
    } else if (!(std::isalnum(c) || (ParseLen_() > 1 && c == '-'))) { // TODO: check this condition...
        LOG(DEBUG) << "Request-Header key can only contain alphanumeric chars and '-' -> bad_request";
        return PS_BAD_REQUEST;
    }
    return PS_HEADER_KEY;
}

RequestBuilder::ParseState RequestBuilder::ParseHeaderKeyValSep_(char c)
{
    LOG(DEBUG) << "Parsing Header-Separator... char: " << c;
    if (std::isspace(c)) {
        return PS_HEADER_KEY_VAL_SEP;
    }
    end_idx_--;
    return PS_HEADER_VALUE;
}
// RFC: https://datatracker.ietf.org/doc/html/rfc2616#page-17
RequestBuilder::ParseState RequestBuilder::ParseHeaderValue_(char c)
{
    LOG(DEBUG) << "Parsing Header-Value... char: " << c << " (" << (int)c << ")";
    if (crlf_counter_ == 0) {
        if (c == '\r') {
            if (ParseLen_() <= 1) {
                return PS_BAD_REQUEST;
            }
            crlf_counter_++;
            return PS_HEADER_VALUE;
        }
        if (!std::isprint(c)) { // TODO: additional checks for valid characters...
            LOG(DEBUG) << "Request-Header value is invalid -> bad_request";
            return PS_BAD_REQUEST;
        }
    } else if (crlf_counter_ == 1) {
        if (c == '\n') {
            LOG(DEBUG) << "HeaderValue complete -> inserting into map...";
            rq_.headers[header_key_] = std::string(buf_.data() + begin_idx_, ParseLen_() - 2);
            crlf_counter_ = 0;
            return PS_BETWEEN_HEADERS;
        }
        else {
            LOG(DEBUG) << "carriage return without line feed found in header-value -> bad_request";
            return PS_BAD_REQUEST;
        }
    }
    return PS_HEADER_VALUE;
}

RequestBuilder::ParseState RequestBuilder::CheckForBody_(void)
{
    LOG(DEBUG) << "After Headers, checking if body is expected...";
    if (rq_.method == HTTP_GET || rq_.method == HTTP_DELETE) {
        LOG(DEBUG) << "No Body expected for GET/DELETE -> request complete";
        rq_.rq_complete = true;
        return PS_END;
    }
    std::string content_length = rq_.GetHeaderVal("content-length");
    std::string transfer_encoding = rq_.GetHeaderVal("transfer-encoding");
    if (!content_length.empty() && !transfer_encoding.empty()) {
        LOG(DEBUG) << "Both Content-Length and Transfer-Encoding found -> Bad Request";
        return PS_BAD_REQUEST;
    }
    if (transfer_encoding == "chunked") {
        LOG(DEBUG) << "Chunked Transfer-Encoding found -> Go read body as chunks";
        rq_.body.chunked = true;
        return PS_BODY;
    }
    if (!content_length.empty()) {
        std::pair<bool, size_t> content_length_num = utils::StrToNumericNoThrow<size_t>(content_length);
        LOG(DEBUG) << "Content-Length found: " << content_length_num.second;
        if (content_length_num.first) {
            LOG(DEBUG) << "Content-lenght is numeric";
            rq_.body.content_length = content_length_num.second;
        } else {
            LOG(DEBUG) << "Content-lenght is non-numeric -> Bad Request";
            return PS_BAD_REQUEST;;
        }
        if (rq_.body.content_length != 0) {
            LOG(DEBUG) << "Content-Length > 0 -> Go read body according to Content-Length";
            return PS_BODY;
        }
    }
    if (rq_.method == HTTP_POST) {
        LOG(DEBUG) << "Method is POST, but no Content-Length or Transfer-Encoding found -> Bad Request";
        return PS_BAD_REQUEST;
    }
    LOG(DEBUG) << "No Content-Length or Transfer-Encoding found -> No Body expected --> request complete...";
    rq_.rq_complete = true;
    return PS_END;
}

// https://datatracker.ietf.org/doc/html/rfc2616#section-3.5
RequestBuilder::ParseState RequestBuilder::ParseBody_(char c)
{
    LOG(DEBUG) << "Todo: Parsing Body... char: " << c;
    rq_.rq_complete = true;
    return PS_END;
}

void RequestBuilder::PrintParseBuf_() const
{
    std::cout << "PrintParseBuf(): Len: " << buf_.size() << "; Content: |";
    for (size_t i = 0; i < buf_.size(); i++) {
        std::cout << buf_[i];
    }
    std::cout << "|" << std::endl;
}

}  // namespace http


// int main() {
//     http::RequestBuilder req_builder;
//     req_builder.buf().resize(1024);
//     std::string raw_req = "GET /index.html HTTP/1.1\r\nKey: Value\r\nHost:localhost\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\n\r\n";
//     std::memcpy(req_builder.buf().data(), raw_req.data(), raw_req.size());
//     req_builder.ParseNext();
//     req_builder.rq().Print();
// }
