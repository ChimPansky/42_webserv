#include "Request.h"

#include <logger.h>
#include <str_utils.h>
#include <sstream>
#include "RqTarget.h"
#include "http.h"

namespace http {

Request::Request() : status(HTTP_OK), method(HTTP_NO_METHOD), version(HTTP_NO_VERSION)
{
    body[0] = 0;
}

Request::~Request() {
    if (has_body && body[0] != 0) {
        std::remove(body);
    }
}

std::pair<bool /*header_key_found*/, std::string /*header_value*/> Request::GetHeaderVal(
    const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(utils::ToLowerCase(key));
    if (it != headers.end()) {
        return std::make_pair(true, it->second);
    }
    return std::make_pair(false, "");
}

std::string Request::GetDebugString() const
{
    std::ostringstream ret;
    ret << "---Request---"
        << "\n\tStatus: " << (status == HTTP_OK ? "OK " : "BAD ") << status
        << "\n\tMethod: " << method
        << "\n\tMethod: " << HttpMethodToStr(method).second
        << "\n\tRequest-Target: " << rqTarget.ToStr()
        << "\n\tVersion: " << HttpVerToStr(version).second
        << "\n\t~Headers~";

    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        ret << "\n\t" << it->first << ": " << it->second;
    }

    ret << "\n\tHas body: " << (has_body ? "Yes" : "No");
    
    return ret.str();
}

}  // namespace http
