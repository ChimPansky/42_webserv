#include "Request.h"

#include "utils/logger.h"

namespace http {

Request::Request()
    : method(HTTP_NO_METHOD), bad_request(false), complete(false), version(HTTP_NO_VERSION), body_size(0)
{}

std::string Request::GetHeaderVal(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

void Request::Print() const
{
    LOG(DEBUG) << "---Request---";
    LOG(DEBUG) << "Bad Request: " << bad_request;
    LOG(DEBUG) << "Complete: " << complete;
    LOG(DEBUG) << "Method: " << method;
    LOG(DEBUG) << "URI: " << uri;
    LOG(DEBUG) << "Version: " << version;
    LOG(DEBUG) << "Host: " << GetHeaderVal("Host");
    LOG(DEBUG) << "User-Agent: " << GetHeaderVal("User-Agent");
    LOG(DEBUG) << "Accept: " << GetHeaderVal("Accept");
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "~Headers~";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        LOG(DEBUG) << "|" << it->first << "|: |" << it->second << "|";
    }
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "Body (TODO): ";
    for (size_t i = 0; i < body.size(); i++) {
        LOG(DEBUG) << body[i];
    }
    LOG(DEBUG) << "Body size (TODO): " << body_size;
}
}  // namespace http
