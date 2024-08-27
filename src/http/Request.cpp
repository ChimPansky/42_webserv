#include "Request.h"

#include <iostream>
namespace http {

const char* httpEOF = "\r\n\r\n";

Request::Request() : _rq_parser(*this) {}

void    Request::AddChunkToRequest(const char* chunk, size_t chunk_sz)
{
   std::cout << "adding chunk to Request: " << chunk << std::endl;
    _raw_request.append(chunk, chunk_sz);
   std::cout << "parsing chunk with RequestParser... " << std::endl;
    _rq_parser.ParseNext(chunk, chunk_sz);

}

const std::string& Request::raw_request() const
{
    return _raw_request;
}

void    Request::Print() const {
    std::cout << "Request Data: " << std::endl;
    std::cout << "_method: " << _method << std::endl;
    std::cout << "_version: " << _version << std::endl;
    std::cout << "_uri: " << _uri << std::endl;
    //...
}

}  // namespace http
