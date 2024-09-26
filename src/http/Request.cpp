#include "Request.h"

#include "utils/logger.h"

namespace http {

void Request::Reset()
{
    bad_request_ = false;
    method = HTTP_NO_METHOD;
    version = HTTP_NO_VERSION;
    uri_.clear();
    host_.clear();
    user_agent_.clear();
    accept_.clear();
    headers_.clear();
    body_.clear();
}

void Request::Print() const
{
    LOG(DEBUG) << "---Request---";
    LOG(DEBUG) << "Bad Request: " << bad_request_;
    LOG(DEBUG) << "Method: " << method;
    LOG(DEBUG) << "URI: " << uri_;
    LOG(DEBUG) << "Version: " << version;
    LOG(DEBUG) << "Host (TODO): " << host_;
    LOG(DEBUG) << "User-Agent(TODO): " << user_agent_;
    LOG(DEBUG) << "Accept(TODO): " << accept_;
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "~Headers~";
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin();
         it != headers_.end(); ++it) {
        LOG(DEBUG) << "|" << it->first << "|: |" << it->second << "|";
    }
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "Body (TODO): ";
    for (size_t i = 0; i < body_.size(); i++) {
        LOG(DEBUG) << body_[i];
    }
    LOG(DEBUG) << "Body size (TODO): " << body_.size();
}
}  // namespace http
