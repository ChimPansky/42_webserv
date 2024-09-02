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

std::vector<char>& RequestBuilder::rq_buf()
{
    return rq_buf_;
}

void RequestBuilder::ParseChunk()
{
    ++chunk_counter_;
    LOG(DEBUG) << "Parsing chunk no " << chunk_counter_ << "...";
    while (parse_idx_ < rq_buf_.size()) {
        char c = rq_buf_[parse_idx_];
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
    if (rq_buf_.size() >= 4 &&
        std::strncmp(rq_buf_.data() + rq_buf_.size() - 4, "\r\n\r\n", 4) == 0) {
        is_request_ready_ = true;
    }
    std::cout << std::endl;
}

void RequestBuilder::Reset()
{
    is_request_ready_ = false;
    chunk_counter_ = 0;
    parse_idx_ = 0;
    rq_buf_.clear();
    parse_state_ = PS_START;
    rq_ = Request();
}

bool RequestBuilder::is_request_ready() const
{
    return is_request_ready_;
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
