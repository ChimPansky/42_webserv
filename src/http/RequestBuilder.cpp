#include "RequestBuilder.h"
#include <iostream>
#include "utils/logger.h"


namespace http {

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

bool RequestBuilder::IsRequestReady() const
{
    return (chunk_counter_ > 5);
}

const Request& RequestBuilder::rq() const
{
    return rq_;
}
}  // namespace http
