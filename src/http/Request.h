#ifndef WS_HTTP_REQUEST_H
#define WS_HTTP_REQUEST_H

#include <string>
#include <map>
#include <vector>

namespace http {
enum Method {
    HTTP_NO_METHOD,
    HTTP_GET,
    HTTP_POST,
    HTTP_DELETE
};
enum Version {  // probably only need to handle Ver_1_0 and Ver_1_1
    HTTP_NO_VERSION,
    HTTP_0_9,
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2,
    HTTP_3
};

struct Request {
    Method method;
    bool bad_request_;
    std::string uri_; // todo: change to struct/class
    Version version;
    std::string host_;
    std::string user_agent_;
    std::string accept_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> params_;
    std::vector<char> body_;
    size_t body_size_;

    void Reset();
    void Print() const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_H
