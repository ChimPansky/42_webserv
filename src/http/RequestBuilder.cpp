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

void RequestBuilder::ParseChunk(const char* chunk, size_t chunk_size)
{
    ++chunk_counter_;
    LOG(DEBUG) << "Parsing chunk no " << chunk_counter_ << "...";
    for (size_t i = 0; i < chunk_size; ++i) {
        char c = chunk[i];
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
    }
    size_t buf_sz = parse_buf_.size();
    parse_buf_.resize(buf_sz + chunk_size);
    std::memcpy(parse_buf_.data() + buf_sz, chunk, chunk_size);
    std::cout << "parse_buf_: " << parse_buf_.data() << std::endl;
    if (parse_buf_.size() >= 4 &&
        std::strncmp(parse_buf_.data() + parse_buf_.size() - 4, "\r\n\r\n", 4) == 0) {
        is_request_ready_ = true;
    }
}

void RequestBuilder::Reset()
{
    is_request_ready_ = false;
    chunk_counter_ = 0;
    parse_buf_.clear();
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

const Request& RequestBuilder::rq() const
{
    return rq_;
}
}  // namespace http
