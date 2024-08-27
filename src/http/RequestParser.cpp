#include "RequestParser.h"

#include <cstddef>
#include <iostream>
namespace http {

RequestParser::RequestParser(Request& rq) : _rq(rq)
{}

void RequestParser::ParseNext(const char* chunk, size_t chunk_sz)
{
    size_t chunk_idx = 0;
    while (chunk_idx < chunk_sz) {
        char c = chunk[chunk_idx];
        switch (_parse_state) {
            case PS_START:
                _parse_str += c;
                _parse_state = PS_METHOD;
                break;
            case PS_METHOD:
                ParseMethod();
                break;
            case PS_URI:
                ParseURI();
                break;
            case PS_VERSION:
                ParseVersion();
                break;
            case PS_HEADERS:
                ParseHeaders();
                break;
            case PS_BODY:
                ParseBody();
                break;
            default:
                break;
        }
        chunk_idx++;
    }
}

void RequestParser::ParseMethod()
{
    // std::cout << "Parsing Method..." << std::endl;
}

void RequestParser::ParseURI()
{
    // std::cout << "Parsing URI..." << std::endl;
}

void RequestParser::ParseVersion()
{
    // std::cout << "Parsing Version..." << std::endl;
}

void RequestParser::ParseHeaders()
{
    // std::cout << "Parsing Headers..." << std::endl;
}

void RequestParser::ParseBody()
{
    // std::cout << "Parsing Body..." << std::endl;
}

void RequestParser::Print() const
{
    std::cout << "_parse_state: " << _parse_state << std::endl;
    std::cout << "_parse_str: " << _parse_str << std::endl;
}

}  // namespace http
