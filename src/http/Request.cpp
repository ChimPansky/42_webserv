#include "Request.h"

#include <iostream>
namespace http {

void Request::AddChunkToRequest(const char* chunk, size_t chunk_sz)
{
    _raw_request.append(chunk, chunk_sz);
    std::cout << "adding chunk: " << chunk << std::endl;
}

const std::string& Request::raw_request() const
{
    return _raw_request;
}

}  // namespace http
