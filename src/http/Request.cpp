#include "Request.h"

#include <iostream>
namespace http {

void    Request::AddChunkToRequest(const char* chunk, size_t chunk_sz)
{
    _raw_request.append(chunk, chunk_sz);
    std::cout << "adding chunk: " << chunk << std::endl;
}

void    Request::ParseNext() {
    while (_parse_idx < _raw_request.size()) {
        char    c = _raw_request.data()[_parse_idx];
        switch(_parse_state) {
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
    }
}

const std::string& Request::raw_request() const
{
    return _raw_request;
}

void    Request::Print() const {
    std::cout << "_parse_state: " << _parse_state << std::endl;
    std::cout << "_parse_idc: " << _parse_idx << std::endl;
    std::cout << "_error: " << _error << std::endl;
}

}  // namespace http
