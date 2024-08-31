#ifndef WS_HTTP_REQUEST_H
#define WS_HTTP_REQUEST_H

#include <string>
#include <map>

namespace http {
enum Method {
    HTTP_GET,
    HTTP_POST,
    HTTP_DELETE
};
enum Version {  // probably only need to handle Ver_1_0 and Ver_1_1
    HTTP_0_9,
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2,
    HTTP_3
};

struct Request {
    Method method;
    Version version;
    std::string uri_; // todo: change to struct/class
    std::string host_;
    std::string user_agent_;
    std::string accept_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> params_;
    std::string body_;
    size_t body_size_;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_H
