#include "Request.h"

#include "utils/logger.h"

namespace http {

std::string ToLowerCase(const std::string& input)  {
    std::string result = input;
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = std::tolower(result[i]);
    }
    return result;
}

Request::Request()
    : method(HTTP_NO_METHOD), version(HTTP_NO_VERSION), bad_request(false), headers_complete(false), rq_complete(false)
{}

Request::Body::Body()
    : chunked(false), chunk_size(0), content_idx(0), remaining_length(0)
{}

bool Request::Body::Complete() const
{
    return remaining_length == 0;
}

std::string Request::GetHeaderVal(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(ToLowerCase(key));
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}


void Request::Print() const
{
    LOG(DEBUG) << "---Request---";
    LOG(DEBUG) << "Method: " << method;
    LOG(DEBUG) << "URI: " << uri;
    LOG(DEBUG) << "Version: " << version;
    LOG(DEBUG) << "Bad Request: " << bad_request;
    LOG(DEBUG) << "Headers complete: " << headers_complete;
    LOG(DEBUG) << "Request complete: " << rq_complete;
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "~Headers~";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        LOG(DEBUG) << "|" << it->first << "|: |" << it->second << "|";
    }
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "Body (TODO): ";
}
}  // namespace http
