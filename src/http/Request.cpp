#include "Request.h"

#include "utils/logger.h"

namespace http {

void Request::Reset() {
    method = HTTP_NO_METHOD;
    version = HTTP_NO_VERSION;
    uri_.clear();
    host_.clear();
    user_agent_.clear();
    accept_.clear();
    headers_.clear();
    params_.clear();
    body_.clear();
    body_size_ = 0;
    status_code_ = 0;
}

void Request::Print() const {
    LOG(DEBUG) << "---Request---";
    LOG(DEBUG) << "Status code: " << status_code_;
    LOG(DEBUG) << "Method: " << method;
    LOG(DEBUG) << "URI: " << uri_;
    LOG(DEBUG) << "Version: " << version;
    LOG(DEBUG) << "Host: " << host_;
    LOG(DEBUG) << "User-Agent: " << user_agent_;
    LOG(DEBUG) << "Accept: " << accept_;
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "~Headers~";
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
        LOG(DEBUG) << "|" << it->first << "|: |" << it->second << "|";
    }
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "~Params~";
    for (std::map<std::string, std::string>::const_iterator it = params_.begin(); it != params_.end(); ++it) {
        LOG(DEBUG) << "|" << it->first << "|: |" << it->second << "|";
    }
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "Body: ";
    for (size_t i = 0; i < body_.size(); i++) {
        LOG(DEBUG) << body_[i];
    }
    LOG(DEBUG) << "Body size: " << body_size_;
}
}  // namespace http
