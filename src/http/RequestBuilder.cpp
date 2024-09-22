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

void RequestBuilder::ParseNext(const std::vector<char>& input, size_t input_sz)
{
    (void)input_sz;
    ++chunk_counter_;
    LOG(DEBUG) << "Parsing chunk no " << chunk_counter_ << "...";
    for (size_t i = 0; i < input_sz; i++) {
        parse_buf_.push_back(input[i]);

    }
    while (parse_idx_ < input.size()) {
        char c = input[parse_idx_];
        (void)c;
        switch (parse_state_) {
            case PS_START:
                break;
            case PS_METHOD:
                break;
            case PS_URI:
                break;
            case PS_VERSION:
                break;
            case PS_HEADERS:
                break;
            case PS_BODY:
                break;
            case PS_END:
                break;
            case PS_ERROR:
                break;
        }
        parse_idx_++;
        std::cout << c;
    }
    if (input.size() >= 4 &&
        std::strncmp(input.data() + input.size() - 4, "\r\n\r\n", 4) == 0) {
        eof_reached_ = true;
    }
    std::cout << std::endl;
}

void RequestBuilder::Reset()
{
    eof_reached_ = false;
    chunk_counter_ = 0;
    parse_idx_ = 0;
    parse_state_ = PS_START;
    rq_ = Request();
}

// ready for response if request has been successfully read until end (eof_reached == false) or a problem has occured during parsing (check request.status_code)
bool RequestBuilder::is_ready_for_response() const
{
    LOG(DEBUG) << "RequestBuilder::is_request_ready: " << (eof_reached_ || rq_.status_code_ != 0);
    return (eof_reached_ || rq_.status_code_ != 0);
}

void RequestBuilder::ParseMethod()
{
    // std::cout << "Parsing Method..." << std::endl;
}

void RequestBuilder::ParseUri()
{
    // std::cout << "Parsing URI..." << std::endl;
}

void RequestBuilder::ParseVersion()
{
    // std::cout << "Parsing Version..." << std::endl;
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
