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
            case kStart:
                _parse_str += c;
                _parse_state = kMethod;
                break;
            case kMethod:
                ParseMethod();
                break;
            case kURI:
                ParseURI();
                break;
            case kVersion:
                ParseVersion();
                break;
            case kHeaders:
                ParseHeaders();
                break;
            case kBody:
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
