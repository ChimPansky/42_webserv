#include "RequestBuilder.h"
#include <cctype>
#include <cstring>
#include "utils/logger.h"
#include <iostream>
#include <vector>
#include <cctype>

namespace http {

RequestBuilder::RequestBuilder()
{
    Reset();
}
const Request& RequestBuilder::rq() const
{
    return rq_;
}

void RequestBuilder::ParseNext(const char* input, size_t input_sz)
{
    ++chunk_counter_;
    LOG(DEBUG) << "Parsing chunk no " << chunk_counter_ << "...";
    for (size_t i = 0; i < input_sz && rq_.status_code_ == 0; i++) {
        char c = input[i];
        parse_buf_.push_back(c);
        switch (parse_state_) {
            case PS_METHOD:
                ParseMethod_();
                break;
            case PS_URI:
                ParseUri_();
                break;
            case PS_VERSION:
                ParseVersion_();
                break;
            case PS_HEADER_KEY:
                break;
            case PS_HEADER_VALUE:
                break;
            case PS_BODY:
                break;
            case PS_END:
                eof_reached_ = true;
                break;
            case PS_ERROR:
                break;
        }
        parse_idx_++;
    }
    if ((input_sz == 0) || (input_sz >= 4 && std::strncmp(input + input_sz - 4, "\r\n\r\n", 4) == 0)) {
        eof_reached_ = true;
        LOG(DEBUG) << "parse_buf_: " << parse_buf_.data();
    }
    if (eof_reached_) {
        CheckIfRequestIsComplete_();
    }

    std::cout << std::endl;
}

void RequestBuilder::Reset()
{
    LOG(DEBUG) << "RequestBuilder::Reset";
    ResetParseBuf_();
    eof_reached_ = false;
    chunk_counter_ = 0;
    parse_state_ = PS_METHOD;
    rq_ = Request();
}

// ready for response if request has been successfully read until end (eof_reached == false) or a problem has occured during parsing (check request.status_code)
bool RequestBuilder::is_ready_for_response() const
{
    LOG(DEBUG) << "RequestBuilder::is_request_ready: " << (eof_reached_ || rq_.status_code_ != 0);
    return (eof_reached_ || rq_.status_code_ != 0);
}

void RequestBuilder::ParseMethod_() {
    LOG(DEBUG) << "Parsing Method..." << "char: " << parse_buf_[parse_idx_];
    bool method_parsed = false;
    if (parse_idx_ == 3 && std::strncmp(parse_buf_.data(), "GET ", 4) == 0) {
        rq_.method = HTTP_GET;
        method_parsed = true;
    } else if (parse_idx_ == 4 && std::strncmp(parse_buf_.data(), "POST ", 5) == 0) {
            rq_.method = HTTP_POST;
            method_parsed = true;
    } else if (parse_idx_ == 6 && std::strncmp(parse_buf_.data(), "DELETE ", 7) == 0) {
            rq_.method = HTTP_DELETE;
            method_parsed = true;
    } else if (parse_idx_ > 6){
            rq_.status_code_ = 400;
            parse_state_ = PS_ERROR;
    }
    if (method_parsed) {
        parse_state_ = PS_URI;
        ResetParseBuf_();
    }
}

void RequestBuilder::ParseUri_() {
    LOG(DEBUG) << "Parsing URI..." << "char: " << parse_buf_[parse_idx_];
    if (parse_idx_ > 0 && parse_buf_[parse_idx_] == ' ') {
        rq_.uri_ = std::string(parse_buf_.data(), parse_idx_ - 1);
        parse_state_ = PS_VERSION;
        ResetParseBuf_();
    }
}

void RequestBuilder::ParseVersion_() {
    LOG(DEBUG) << "Parsing Version..." << "char: " << parse_buf_[parse_idx_];
    bool version_parsed = false;
    if (parse_idx_ == 9) {
        LOG(DEBUG) << "parse_buf_: " << std::string( parse_buf_.data(), 10 );
    }
    if (parse_idx_ == 9 && std::strncmp(parse_buf_.data(), "HTTP/0.9\r\n", 10) == 0) {
        rq_.version = HTTP_0_9;
        version_parsed = true;
    }
    else if (parse_idx_ == 9 && std::strncmp(parse_buf_.data(), "HTTP/1.0\r\n", 10) == 0) {
        rq_.version = HTTP_1_0;
        version_parsed = true;
    } else if (parse_idx_ == 9 && std::strncmp(parse_buf_.data(), "HTTP/1.1\r\n", 10) == 0) {
        rq_.version = HTTP_1_1;
        version_parsed = true;
    } else if (parse_idx_ == 7 && std::strncmp(parse_buf_.data(), "HTTP/2\r\n", 8) == 0) {
        rq_.version = HTTP_2;
        version_parsed = true;
    } else if (parse_idx_ == 7 && std::strncmp(parse_buf_.data(), "HTTP/3\r\n", 8) == 0) {
        rq_.version = HTTP_3;
        version_parsed = true;
    }
    if (version_parsed) {
        parse_state_ = PS_HEADER_KEY;
        ResetParseBuf_();
    } else if (parse_idx_ > 9) {
        rq_.status_code_ = 400;
        parse_state_ = PS_ERROR;
    }
}

void RequestBuilder::ParseHeaderKey_() {
    LOG(DEBUG) << "Parsing Header-Key...";
    char c = parse_buf_[parse_idx_];
    if (LineIsEmpty_()) {
        parse_state_ = PS_BODY;
        return;
    }
    if (c == ':') {
        if (parse_idx_ == 0 || parse_buf_[parse_idx_ - 1] == '-') {
            rq_.status_code_ = 400;
            parse_state_ = PS_ERROR;
        }
        else {
            header_key_ = std::string(parse_buf_.data(), parse_idx_);
            parse_state_ = PS_HEADER_VALUE;
            ResetParseBuf_();
        }
    }
    else if (!(std::isalnum(c) || (parse_idx_ > 0 && c == '-'))) {
        rq_.status_code_ = 400;
        parse_state_ = PS_ERROR;
        LOG(DEBUG) << "Request-Header can only contain alphanumeric chars and '-'";
    }
    if (rq_.status_code_ != 0) {
        LOG(ERROR) << "Request-Header key is invalid";
    }
}

void RequestBuilder::ParseHeaderValue_() {
    LOG(DEBUG) << "Parsing Header-Value...";
    char c = parse_buf_[parse_idx_];
    if (std::isspace(c)) {
        return;
    }
    if (!std::isprint(c)) {
        rq_.status_code_ = 400;
        parse_state_ = PS_ERROR;
        LOG(DEBUG) << "Request-Header value is invalid";
    }
    if (parse_idx_ > 2 && std::strncmp(parse_buf_.data() + parse_idx_ - 2, "\r\n", 2) == 0) {
        rq_.headers_[header_key_] = std::string(parse_buf_.data(), parse_idx_ - 2);
        parse_state_ = PS_HEADER_KEY;
        ResetParseBuf_();
    }
}

void RequestBuilder::ParseBody_()
{
    LOG(DEBUG) << "Parsing Body...";
    parse_state_ = PS_END;
}

void RequestBuilder::CheckIfRequestIsComplete_()
{
    if (parse_state_ != PS_END) {
        rq_.status_code_ = 400;
    }
}

void RequestBuilder::ResetParseBuf_() {
    LOG(DEBUG) << "RequestBuilder::ResetParseBuf";
    parse_buf_.clear();
    parse_idx_ = 0;
}

bool RequestBuilder::LineIsEmpty_()
{
    if (parse_idx_ == 1 && parse_buf_[0] == 'r' && parse_buf_[1] == '\n') {
        return true;
    }
    return false;
}

}  // namespace http
