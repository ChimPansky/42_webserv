#include "Request.h"

#include <logger.h>
#include <str_utils.h>
#include "ResponseCodes.h"

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

void Request::Print() const
{
    LOG(DEBUG) << "---Request---";
    LOG(DEBUG) << "Status: "
               << (status == RQ_INCOMPLETE ? "Incomplete" : (status == RQ_GOOD ? "Good" : "Bad"));
    LOG(DEBUG) << "Method: " << method;
    LOG(DEBUG) << "URI: " << uri;
    LOG(DEBUG) << "Version: " << version;
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "~Headers~";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        LOG(DEBUG) << "|" << it->first << "|: |" << it->second << "|";
    }
    LOG(DEBUG) << "Body size: " << body.size();
    LOG(DEBUG) << "Body: ";
    BodyPrint();
    LOG(DEBUG) << "\n";
}

void Request::BodyPrint() const {
    for (std::vector<char>::const_iterator it = body.begin(); it != body.end(); ++it) {
        LOG(DEBUG) << *it;
    }
}

}  // namespace http
