#ifndef WS_HTTP_REQUEST_H
#define WS_HTTP_REQUEST_H

#include <map>
#include <string>
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
    Request();

    Method method;
    bool bad_request;
    bool complete;
    std::string uri;  // todo: change to struct/class
    Version version;
    std::string host;
    std::string user_agent;
    std::string accept;
    std::map<std::string, std::string> headers;
    std::vector<char> body;
    size_t body_size;

    void Reset();
    void Print() const;
    std::string GetHeaderVal(const std::string& key) const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_H
