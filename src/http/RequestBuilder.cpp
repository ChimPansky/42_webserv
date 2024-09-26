#include "RequestBuilder.h"
#include <cctype>
#include <cstring>
#include "utils/logger.h"
#include <iostream>
#include <vector>
#include <cctype>

namespace http {

RequestBuilder::EOFChecker::EOFChecker() {
    Reset();
}

void RequestBuilder::EOFChecker::Reset() {
    counter_ = 0;
    end_of_line_ = false;
    end_of_file_ = false;
}

void RequestBuilder::EOFChecker::Update(char c) {
    if (counter_ == 0 || counter_ == 2) {
        if (c == '\r') {
            counter_++;
        }
        else {
            counter_ = 0;
        }
    }
    else if (counter_ == 1 || counter_ == 3) {
        if (c == '\n') {
            counter_++;
        }
        else {
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
{
    Reset();
}
const Request& RequestBuilder::rq() const {
    return rq_;
}

void RequestBuilder::ParseNext(const char* input, size_t input_sz) {
    ++chunk_counter_;
    //LOG(DEBUG) << "Parsing chunk no " << chunk_counter_ << "; bytes_received: " << input_sz;
    if (input_sz == 0) {
        rq_.bad_request_ = true;
    }
    for (size_t i = 0; i < input_sz && !IsReadyForResponse(); i++) {
        char c = input[i];
        parse_buf_.push_back(c);
        parse_buf_len_++;
        //PrintParseBuf_();
        eof_checker_.Update(c);
        if (eof_checker_.end_of_file_ && (parse_state_ == PS_METHOD || parse_state_ == PS_URI
            || parse_state_ == PS_VERSION/*needs to have at least 1 header?*/ || parse_state_ == PS_HEADER_SEP)) {
            rq_.bad_request_ = true;
            continue;
        }
        bool state_changed = false;
        switch (parse_state_) {
            case PS_METHOD:
                parse_state_ = ParseMethod_();
                state_changed = (parse_state_ != PS_METHOD ? true : false);
                break;
            case PS_URI:
                parse_state_ = ParseUri_();
                state_changed = (parse_state_ != PS_URI ? true : false);
                break;
            case PS_VERSION:
                parse_state_ = ParseVersion_();
                state_changed = (parse_state_ != PS_VERSION ? true : false);
                break;
            case PS_HEADER_KEY:
                parse_state_ = ParseHeaderKey_();
                state_changed = (parse_state_ != PS_HEADER_KEY ? true : false);
                break;
            case PS_HEADER_SEP:
                parse_state_ = ParseHeaderSep_();
                state_changed = (parse_state_ != PS_HEADER_SEP ? true : false);
                break;
            case PS_HEADER_VALUE:
                parse_state_ = ParseHeaderValue_();
                state_changed = (parse_state_ != PS_HEADER_VALUE ? true : false);
                break;
            case PS_BODY:
                parse_state_ = ParseBody_();
                state_changed = (parse_state_ != PS_BODY ? true : false);
                break;
            case PS_END:
                break;
            case PS_ERROR:
                break;
        }
        if (state_changed) {
            ResetParseBuf_();
        }
    }
    if (eof_checker_.end_of_file_ && parse_state_ != PS_END) {
        rq_.bad_request_ = true;
    }
    std::cout << std::endl;
}

void RequestBuilder::Reset() {
    //LOG(DEBUG) << "RequestBuilder::Reset";
    eof_checker_.Reset();
    ResetParseBuf_();
    chunk_counter_ = 0;
    parse_state_ = PS_METHOD;
    rq_ = Request();
}

// ready for response if request has been successfully read until end (eof_reached == false) or a problem has occured during parsing (check request.status_code)
bool RequestBuilder::IsReadyForResponse() {
    //LOG(DEBUG) << "RequestBuilder::IsReadyForResponse: EOF: " << eof_checker_.end_of_file_ << "; Bad Request: " << rq_.bad_request_;
    return (eof_checker_.end_of_file_ || rq_.bad_request_ /*body_complete()*/);
}

RequestBuilder::ParseState RequestBuilder::ParseMethod_() {
    //LOG(DEBUG) << "Parsing Method..." << "char: " << parse_buf_[parse_buf_len_ - 1];
    if (parse_buf_len_ == 4 && std::strncmp(parse_buf_.data(), "GET ", 4) == 0) {
        rq_.method = HTTP_GET;
        return PS_URI;
    }
    if (parse_buf_len_ == 5 && std::strncmp(parse_buf_.data(), "POST ", 5) == 0) {
            rq_.method = HTTP_POST;
            return PS_URI;
    }
    if (parse_buf_len_ == 7 && std::strncmp(parse_buf_.data(), "DELETE ", 7) == 0) {
            rq_.method = HTTP_DELETE;
            return PS_URI;
    }
    if (parse_buf_len_ > 7){
            rq_.bad_request_ = true;
            return PS_ERROR;
    }
    return PS_METHOD;
}

RequestBuilder::ParseState RequestBuilder::ParseUri_() {
    //LOG(DEBUG) << "Parsing URI..." << "parse_buf_[" << parse_buf_len_ << "]: " << parse_buf_[parse_buf_len_ - 1];

    if (parse_buf_len_ > 0 && parse_buf_[parse_buf_len_ - 1] == ' ') {
        rq_.uri_ = std::string(parse_buf_.data(), parse_buf_len_ - 1);
        return PS_VERSION;
    }
    return PS_URI;
}

RequestBuilder::ParseState RequestBuilder::ParseVersion_() {
    //LOG(DEBUG) << "Parsing Version..." << "char: " << parse_buf_[parse_buf_len_ - 1];
    if (parse_buf_len_ == 10 && std::strncmp(parse_buf_.data(), "HTTP/0.9\r\n", 10) == 0) {
        rq_.version = HTTP_0_9;
        return PS_HEADER_KEY;
    }
    if (parse_buf_len_ == 10 && std::strncmp(parse_buf_.data(), "HTTP/1.0\r\n", 10) == 0) {
        rq_.version = HTTP_1_0;
        return PS_HEADER_KEY;
    }
    if (parse_buf_len_ == 10 && std::strncmp(parse_buf_.data(), "HTTP/1.1\r\n", 10) == 0) {
        rq_.version = HTTP_1_1;
        return PS_HEADER_KEY;
    }
    if (parse_buf_len_ == 8 && std::strncmp(parse_buf_.data(), "HTTP/2\r\n", 8) == 0) {
        rq_.version = HTTP_2;
        return PS_HEADER_KEY;
    }
    if (parse_buf_len_ == 8 && std::strncmp(parse_buf_.data(), "HTTP/3\r\n", 8) == 0) {
        rq_.version = HTTP_3;
        return PS_HEADER_KEY;
    }
    if (parse_buf_len_ > 10) {
        rq_.bad_request_ = true;
        return PS_ERROR;
    }
    return PS_VERSION;
}

RequestBuilder::ParseState RequestBuilder::ParseHeaderKey_() {
    //LOG(DEBUG) << "Parsing Header-Key...";
    char c = parse_buf_[parse_buf_len_ - 1];
    if (eof_checker_.end_of_file_) {
        //LOG(DEBUG) << "Found EOF while trying go read Headerkey -> End of request (Needed to read body?) ";
        return PS_END;
    }
    if (LineIsEmpty_()) {
        //LOG(DEBUG) << "--EMPTY LINE-- switching to PS_BODY";
        return PS_BODY;
    }
    if (c == ':') {
        if (parse_buf_len_ == 0 || parse_buf_[parse_buf_len_ - 2] == '-') {
            rq_.bad_request_ = true;
        }
        else {
            header_key_ = std::string(parse_buf_.data(), parse_buf_len_ - 1);
            return PS_HEADER_SEP;
        }
    }
    else if (!(std::isalnum(c) || (parse_buf_len_ > 0 && c == '-')) && c != '\r') {
        rq_.bad_request_ = true;
        //LOG(DEBUG) << "Request-Header can only contain alphanumeric chars and '-'";
    }
    if (rq_.bad_request_) {
        LOG(ERROR) << "Request-Header key is invalid";
        return PS_ERROR;
    }
    return PS_HEADER_KEY;
}

RequestBuilder::ParseState RequestBuilder::ParseHeaderSep_() {
    //LOG(DEBUG) << "Parsing Header-Separator...";
    char c = parse_buf_[parse_buf_len_ - 1];
    if (std::isspace(c)) {
        return PS_HEADER_SEP;
    }
    return PS_HEADER_VALUE;
}

RequestBuilder::ParseState RequestBuilder::ParseHeaderValue_() {
    //LOG(DEBUG) << "Parsing Header-Value...";
    char c = parse_buf_[parse_buf_len_ - 1];
    if (parse_buf_len_ > 3 && eof_checker_.end_of_line_) {
        //LOG(DEBUG) << "HeaderValue complete -> inserting into map...";
        rq_.headers_[header_key_] = std::string(parse_buf_.data(), parse_buf_len_ - 2);
        return PS_HEADER_KEY;
    }
    if (!std::isprint(c) && c != '\r') {
        rq_.bad_request_ = true;
        //LOG(DEBUG) << "Request-Header value is invalid";
        return PS_ERROR;
    }
    return PS_HEADER_VALUE;
}

RequestBuilder::ParseState RequestBuilder::ParseBody_() {
    //LOG(DEBUG) << "Parsing Body...";
    if (eof_checker_.end_of_file_) {
        rq_.bad_request_ = true;
        return PS_BODY;
    }
    return PS_END;
}

void RequestBuilder::ResetParseBuf_() {
    //LOG(DEBUG) << "RequestBuilder::ResetParseBuf";
    if (parse_state_ == PS_HEADER_VALUE) {
        char beginning_of_header_value = parse_buf_[parse_buf_len_ - 1];
        parse_buf_.clear();
        parse_buf_.push_back(beginning_of_header_value);
        parse_buf_len_ = 1;
    }
    else {
        parse_buf_.clear();
        parse_buf_len_ = 0;
    }
}

void RequestBuilder::PrintParseBuf_() const {
    std::cout << "PrintParseBuf(): Len: " << parse_buf_len_ << "; Content: |";
    for (size_t i = 0; i < parse_buf_.size(); i++) {
        std::cout << parse_buf_[i];
    }
    std::cout << "|" << std::endl;
}

bool RequestBuilder::LineIsEmpty_() const {
    //LOG(DEBUG) << "LineIsEmpty()";
    if (parse_buf_len_ == 2 && eof_checker_.end_of_line_) {
        //LOG(DEBUG) << "LineIEmpty(): empty line found";
        return true;
    }
    return false;
}

}  // namespace http
