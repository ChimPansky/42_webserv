#include "Request.h"

#include <logger.h>
#include <str_utils.h>
#include <sstream>

namespace http {

Request::Request() : status(RQ_INCOMPLETE), method(HTTP_NO_METHOD), version(HTTP_NO_VERSION)
{}

std::pair<bool /*header_key_found*/, std::string /*header_value*/> Request::GetHeaderVal(
    const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(utils::ToLowerCase(key));
    if (it != headers.end()) {
        return std::make_pair(true, it->second);
    }
    return std::make_pair(false, "");
}

std::string Request::ToString() const
{
    std::ostringstream ret;
    ret << "---Request---\n"
        << "Status: " << (status == RQ_INCOMPLETE ? "Incomplete" : (status == RQ_BAD ? "Bad" : "Good")) << "\n"
        << "Method: " << method << "\n"
        << "URI: " << uri << "\n"
        << "Version: " << version << "\n"
        << "~Headers~\n";

    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        ret << it->first << ": " << it->second << "\n";
    }

    ret << "Body size: " << body.size() << "\n"
        << "Body: " << std::string(body.data(), body.size()) << "\n";

    return ret.str();
}

}  // namespace http
