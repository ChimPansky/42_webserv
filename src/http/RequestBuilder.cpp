#include "RequestBuilder.h"
#include <cstring>
#include "utils/logger.h"
#include <iostream>
#include <vector>

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
        parse_idx_++;
        switch (parse_state_) {
            case PS_METHOD:
                ParseMethod();
                break;
            case PS_URI:
                ParseUri();
                break;
            case PS_VERSION:
                ParseVersion();
                break;
            case PS_HEADERS:
                break;
            case PS_BODY:
                break;
            case PS_END:
                eof_reached_ = true;
                break;
            case PS_ERROR:
                break;
        }
        std::cout << c;
    }
    if ((input_sz == 0) || (input_sz >= 4 && std::strncmp(input + input_sz - 4, "\r\n\r\n", 4) == 0)) {
        eof_reached_ = true;
        LOG(DEBUG) << "parse_buf_: " << parse_buf_.data();
    }
    if (eof_reached_) {
        CheckIfRequestIsComplete();
    }

    std::cout << std::endl;
}

void RequestBuilder::CheckIfRequestIsComplete()
{
    if (parse_state_ != PS_END) {
        rq_.status_code_ = 400;
    }
}

void RequestBuilder::Reset()
{
    LOG(DEBUG) << "RequestBuilder::Reset";
    ResetParseBuf();
    eof_reached_ = false;
    chunk_counter_ = 0;
    parse_state_ = PS_METHOD;
    rq_ = Request();
}

void RequestBuilder::ResetParseBuf() {
    LOG(DEBUG) << "RequestBuilder::ResetParseBuf";
    parse_buf_.clear();
    parse_idx_ = 0;
}

// ready for response if request has been successfully read until end (eof_reached == false) or a problem has occured during parsing (check request.status_code)
bool RequestBuilder::is_ready_for_response() const
{
    LOG(DEBUG) << "RequestBuilder::is_request_ready: " << (eof_reached_ || rq_.status_code_ != 0);
    return (eof_reached_ || rq_.status_code_ != 0);
}

void RequestBuilder::ParseMethod() {
    LOG(DEBUG) << "Parsing Method...";
    bool method_parsed = false;
    if (parse_idx_ == 4 && std::strncmp(parse_buf_.data(), "GET ", 4) == 0) {
        rq_.method = HTTP_GET;
        method_parsed = true;
    } else if (parse_idx_ == 5 && std::strncmp(parse_buf_.data(), "POST ", 5) == 0) {
            rq_.method = HTTP_POST;
            method_parsed = true;
    } else if (parse_idx_ == 7 && std::strncmp(parse_buf_.data(), "DELETE ", 7) == 0) {
            rq_.method = HTTP_DELETE;
            method_parsed = true;
    } else if (parse_idx_ > 7){
            rq_.status_code_ = 400;
            parse_state_ = PS_ERROR;
    }
    if (method_parsed) {
        parse_state_ = PS_URI;
        ResetParseBuf();
    }
}

void RequestBuilder::ParseUri() {
    LOG(DEBUG) << "Parsing URI...";
    if (parse_idx_ > 0 && parse_buf_[parse_idx_ - 1] == ' ') {
        rq_.uri_ = std::string(parse_buf_.data(), parse_idx_ - 1);
        parse_state_ = PS_VERSION;
        ResetParseBuf();
    }
}

void RequestBuilder::ParseVersion() {
    LOG(DEBUG) << "Parsing Version...";
    bool version_parsed = false;
    if (parse_idx_ == 10 && std::strncmp(parse_buf_.data(), "HTTP/0.9\r\n", 10) == 0) {
        rq_.version = HTTP_0_9;
        version_parsed = true;
    }
    else if (parse_idx_ == 10 && std::strncmp(parse_buf_.data(), "HTTP/1.0\r\n", 10) == 0) {
        rq_.version = HTTP_1_0;
        version_parsed = true;
    } else if (parse_idx_ == 10 && std::strncmp(parse_buf_.data(), "HTTP/1.1\r\n", 10) == 0) {
        rq_.version = HTTP_1_1;
        version_parsed = true;
    } else if (parse_idx_ == 8 && std::strncmp(parse_buf_.data(), "HTTP/2\r\n", 8) == 0) {
        rq_.version = HTTP_2;
        version_parsed = true;
    } else if (parse_idx_ == 8 && std::strncmp(parse_buf_.data(), "HTTP/3\r\n", 8) == 0) {
        rq_.version = HTTP_3;
        version_parsed = true;
    }
    if (version_parsed) {
        parse_state_ = PS_END;
        ResetParseBuf();
    } else if (parse_idx_ > 10) {
        rq_.status_code_ = 400;
        parse_state_ = PS_ERROR;
    }
}

void RequestBuilder::ParseHeaders()
{
    // std::cout << "Parsing Headers..." << std::endl;
}

void RequestBuilder::ParseBody()
{
    // std::cout << "Parsing Body..." << std::endl;
}

}  // namespace http
