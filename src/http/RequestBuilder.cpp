#include "RequestBuilder.h"

#include <cctype>
#include <cstring>
#include <iostream>
#include <vector>

#include "utils/logger.h"
#include "utils/utils.h"

namespace http {

RequestBuilder::EOFChecker::EOFChecker() : counter_(0), end_of_line_(false), end_of_file_(false)
{}
void RequestBuilder::EOFChecker::Update(char c)
{
    if (counter_ == 0 && c == '\r') {
        counter_++;
    } else if (counter_ == 1) {
        if (c == '\n') {
            counter_++;
        } else if (c != '\r') {
            counter_ = 0;
        }
    } else if (counter_ == 2) {
        if (c == '\r') {
            counter_++;
        } else {
            counter_ = 0;
        }
    } else if (counter_ == 3) {
        if (c == '\r') {
            counter_ = 1;
        } else if (c == '\n') {
            counter_++;
        } else {
            counter_ = 0;
        }
    }
    end_of_line_ = (counter_ == 2);
    if (end_of_line_) {
        //LOG(DEBUG) << "---EOL FOUND!---";
    }
    end_of_file_ = (counter_ == 4);
    if (end_of_file_) {
        //LOG(DEBUG) << "---EOF FOUND!---";
    }
}

RequestBuilder::RequestBuilder()
    : chunk_counter_(0), begin_idx_(0), end_idx_(0), parse_state_(PS_METHOD)
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
    if (buf_.size() == end_idx_) {  // 0 bytes received...
        rq_.bad_request = true;
    }
    while (end_idx_ < buf_.size() && !IsReadyForResponse()) {
        char c = buf_[end_idx_++];
        // PrintParseBuf_();
        //LOG(DEBUG) << "Parsing char: " << (int)c;
        eof_checker_.Update(c);
        rq_.rq_complete = eof_checker_.end_of_file_;
        if (eof_checker_.end_of_file_ &&
            (parse_state_ == PS_METHOD || parse_state_ == PS_URI ||
             parse_state_ == PS_VERSION /*needs to have at least 1 header?*/ ||
             parse_state_ == PS_HEADER_SEP)) {
            rq_.bad_request = true;
            continue;
        }
        bool state_changed = false;
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
            case PS_HEADER_KEY:
                parse_state_ = ParseHeaderKey_(c);
                state_changed = (parse_state_ != PS_HEADER_KEY ? true : false);
                break;
            case PS_HEADER_SEP:
                parse_state_ = ParseHeaderSep_(c);
                state_changed = (parse_state_ != PS_HEADER_SEP ? true : false);
                break;
            case PS_HEADER_VALUE:
                parse_state_ = ParseHeaderValue_(c);
                state_changed = (parse_state_ != PS_HEADER_VALUE ? true : false);
                break;
            case PS_AFTER_HEADERS:
                parse_state_ = ParseHeaderValue_(c);
                state_changed = (parse_state_ != PS_HEADER_VALUE ? true : false);
                break;
            case PS_BODY:
                parse_state_ = ParseBody_(c);
                state_changed = (parse_state_ != PS_BODY ? true : false);
                break;
            case PS_END:
                break;
            case PS_ERROR:
                break;
        }
        if (state_changed) {
            if (parse_state_ == PS_AFTER_HEADERS) {
                GetBodySettingsFromHeaders_();
            }
            UpdateBeginIdx_();
        }
    }
    if (eof_checker_.end_of_file_ && parse_state_ != PS_END) {
        rq_.bad_request = true;
    }
    std::cout << std::endl;
}

bool RequestBuilder::IsReadyForResponse()
{
    //LOG(DEBUG) << "RequestBuilder::IsReadyForResponse: EOF: " << eof_checker_.end_of_file_
    //           << "Bad Request: " << rq_.bad_request_;
    return (eof_checker_.end_of_file_ || rq_.bad_request /*body_complete()*/);
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
        return PS_ERROR;
    }
    return PS_METHOD;
}

RequestBuilder::ParseState RequestBuilder::ParseUri_(char c)
{
    //LOG(DEBUG) << "Parsing URI... char: " << c;

    if (c == ' ') {
        if (ParseLen_() > 1) {
            rq_.uri = std::string(buf_.data() + begin_idx_, buf_.data() + end_idx_ - 1);
            return PS_VERSION;
        } else {
            rq_.bad_request = true;
            return PS_ERROR;
        }
    }
    return PS_URI;
}

RequestBuilder::ParseState RequestBuilder::ParseVersion_(char c)
{
    (void)c;
    //LOG(DEBUG) << "Parsing Version... char: " << c;
    if (ParseLen_() == 10 && CompareBuf_("HTTP/0.9\r\n", 10) == 0) {
        rq_.version = HTTP_0_9;
        return PS_HEADER_KEY;
    }
    if (ParseLen_() == 10 && CompareBuf_("HTTP/1.0\r\n", 10) == 0) {
        rq_.version = HTTP_1_0;
        return PS_HEADER_KEY;
    }
    if (ParseLen_() == 10 && CompareBuf_("HTTP/1.1\r\n", 10) == 0) {
        rq_.version = HTTP_1_1;
        return PS_HEADER_KEY;
    }
    if (ParseLen_() == 8 && CompareBuf_("HTTP/2\r\n", 8) == 0) {
        rq_.version = HTTP_2;
        return PS_HEADER_KEY;
    }
    if (ParseLen_() == 8 && CompareBuf_("HTTP/3\r\n", 8) == 0) {
        rq_.version = HTTP_3;
        return PS_HEADER_KEY;
    }
    if (ParseLen_() > 10) {
        rq_.bad_request = true;
        return PS_ERROR;
    }
    return PS_VERSION;
}

RequestBuilder::ParseState RequestBuilder::ParseHeaderKey_(char c)
{
    //LOG(DEBUG) << "Parsing Header-Key... char: " << c;
    if (eof_checker_.end_of_file_) {
        //LOG(DEBUG)
        //    << "Found EOF while trying go read Headerkey -> End of request (Needed to read body?) ";
        return PS_END;
    }
    if (LineIsEmpty_()) {
        //LOG(DEBUG) << "--EMPTY LINE-- switching to PS_BODY";
        rq_.headers_complete = true;
        return PS_AFTER_HEADERS;
    }
    if (c == ':') {
        if (ParseLen_() == 1) {
            rq_.bad_request = true;
        } else {
            header_key_ = std::string(buf_.data() + begin_idx_, ParseLen_() - 1);
            return PS_HEADER_SEP;
        }
    } else if (!(std::isalnum(c) || (ParseLen_() > 0 && c == '-')) && c != '\r') {
        rq_.bad_request = true;
        //LOG(DEBUG) << "Request-Header can only contain alphanumeric chars and '-'";
    }
    if (rq_.bad_request) {
        LOG(ERROR) << "Request-Header key is invalid: " << std::string(buf_.data() + begin_idx_, ParseLen_() - 1);
        return PS_ERROR;
    }
    return PS_HEADER_KEY;
}

RequestBuilder::ParseState RequestBuilder::ParseHeaderSep_(char c)
{
    //LOG(DEBUG) << "Parsing Header-Separator... char: " << c;
    if (std::isspace(c)) {
        return PS_HEADER_SEP;
    }
    end_idx_--;
    return PS_HEADER_VALUE;
}

RequestBuilder::ParseState RequestBuilder::ParseHeaderValue_(char c)
{
    //LOG(DEBUG) << "Parsing Header-Value... char: " << c;
    if (ParseLen_() > 2 && eof_checker_.end_of_line_) {
        //LOG(DEBUG) << "HeaderValue complete -> inserting into map...";
        rq_.headers[header_key_] = std::string(buf_.data() + begin_idx_, ParseLen_() - 2);
        return PS_HEADER_KEY;
    }
    if (!std::isprint(c) && c != '\r') {
        rq_.bad_request = true;
        //LOG(DEBUG) << "Request-Header value is invalid";
        return PS_ERROR;
    }
    return PS_HEADER_VALUE;
}

RequestBuilder::ParseState RequestBuilder::ParseAfterHeaders_(char c)
{
    (void)c;
    //LOG(DEBUG) << "Parsing After-Headers... char: " << c;
    if (ParseLen_() == 2 && eof_checker_.end_of_file_) {
        return PS_END;
        return PS_BODY;
    }
    return PS_AFTER_HEADERS;
}

RequestBuilder::ParseState RequestBuilder::ParseBody_(char c)
{
    (void)c;
    //LOG(DEBUG) << "Parsing Body... char: " << c;
    if (eof_checker_.end_of_file_) {
        rq_.bad_request = true;
        return PS_BODY;
    }
    return PS_END;
}

void RequestBuilder::GetBodySettingsFromHeaders_()
{
    //LOG(DEBUG) << "RequestBuilder::GetBodySettingsFromHeaders_";
    std::string content_type = rq_.GetHeaderVal("Content-Type");
    std::string transfer_encoding = rq_.GetHeaderVal("Transfer-Encoding");
    std::string content_length = rq_.GetHeaderVal("Content-Length");
    if (content_type == "application/json") {
        //LOG(DEBUG) << "Content-Type: application/json";
    }
    if (transfer_encoding == "chunked") {
        //LOG(DEBUG) << "Transfer-Encoding: chunked";
        rq_.body.chunked = true;
    }
    if (!content_length.empty()) {
        //LOG(DEBUG) << "Content-Length: " << content_length;
        std::pair<bool, size_t> content_length_num = utils::StrToNumericNoThrow<size_t>(content_length);
        if (content_length_num.first) {
            rq_.body.size = content_length_num.second;
        } else {
            rq_.bad_request = true;
            return;
        }
    }
}

size_t RequestBuilder::ParseLen_() const
{
    return end_idx_ - begin_idx_;
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

void RequestBuilder::PrintParseBuf_() const
{
    std::cout << "PrintParseBuf(): Len: " << buf_.size() << "; Content: |";
    for (size_t i = 0; i < buf_.size(); i++) {
        std::cout << buf_[i];
    }
    std::cout << "|" << std::endl;
}

bool RequestBuilder::LineIsEmpty_() const
{
    //LOG(DEBUG) << "LineIsEmpty()";
    if (ParseLen_() == 2 && eof_checker_.end_of_line_) {
        //LOG(DEBUG) << "LineIEmpty(): empty line found";
        return true;
    }
    return false;
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
