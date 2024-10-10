#include "Request.h"

#include "../utils/logger.h"
#include "../utils/utils.h"

namespace http {

Request::Request()
    : status_(RQ_INCOMPLETE), method_(HTTP_NO_METHOD), version_(HTTP_NO_VERSION)
{}

std::pair<bool/*header-key found*/, std::string /*header-value*/> Request::GetHeaderVal(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers_.find(utils::ToLowerCase(key));
    if (it != headers_.end()) {
        return std::make_pair(true, it->second);
    }
    return std::make_pair(false, "");
}

void Request::Print() const
{
    LOG(DEBUG) << "---Request---";
    LOG(DEBUG) << "Status: " << (status_ == RQ_INCOMPLETE ? "Incomplete" : (status_ == RQ_BAD ? "Bad" : "Good"));
    LOG(DEBUG) << "Method: " << method_;
    LOG(DEBUG) << "URI: " << uri_;
    LOG(DEBUG) << "Version: " << version_;
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "~Headers~";
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin();
         it != headers_.end(); ++it) {
        LOG(DEBUG) << "|" << it->first << "|: |" << it->second << "|";
    }
    LOG(DEBUG) << "Body size: " << body_.size();
    LOG(DEBUG) << "Body: " << body_.data();
    LOG(DEBUG) << "\n";
}

//Getters:
RQ_Status Request::status() const
{
    return status_;
}

Method Request::method() const
{
    return method_;
}

const std::string& Request::uri() const
{
    return uri_;
}

Version Request::version() const
{
    return version_;
}

const std::map<std::string, std::string>& Request::headers() const
{
    return headers_;
}

const std::vector<char>& Request::body() const
{
    return body_;
}

}  // namespace http
