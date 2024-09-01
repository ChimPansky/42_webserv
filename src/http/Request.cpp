#include "Request.h"

namespace http {

void Request::Reset()
{
    method = HTTP_GET;
    version = HTTP_1_1;
    uri_.clear();
    host_.clear();
    user_agent_.clear();
    accept_.clear();
    headers_.clear();
    params_.clear();
    body_.clear();
    body_size_ = 0;
}

}  // namespace http

